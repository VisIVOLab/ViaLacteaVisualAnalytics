#include "authwrapper.h"
#include "vlvaurlschemehandler.h"

#include <QTimer>
#include <QtNetwork>
#include <QWebEngineProfile>

AuthWrapper::AuthWrapper(QObject *parent) : QObject(parent)
{
    mgr = new QNetworkAccessManager(this);

    // Install vlva:// scheme handler
    auto handler = new VLVAUrlSchemeHandler(this);
    connect(handler, &VLVAUrlSchemeHandler::codeReceived, this, &AuthWrapper::exchangeTokenFromCode);
    QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("vlva", handler);

    // Setup OAuth2
    auto replyHandler = new CustomOAuthReplyHandler(this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(authUrl);
    oauth2.setAccessTokenUrl(tokenUrl);
    oauth2.setClientIdentifier(clientId);
    oauth2.setClientIdentifierSharedKey(clientSecret);
    oauth2.setScope(scope);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &AuthWrapper::open_webview);
    connect(this, &AuthWrapper::authenticated, this, &AuthWrapper::on_authenticated);
    connect(this, &AuthWrapper::refresh_timeout, this, &AuthWrapper::on_refresh_timeout);
}

void AuthWrapper::grant()
{
    oauth2.grant();
}

void AuthWrapper::open_webview(const QUrl & url)
{
    view = new QWebEngineView();
    view->load(url);
    view->setContextMenuPolicy(Qt::NoContextMenu);
    view->resize(1024, 768);
    view->show();
}

void AuthWrapper::exchangeTokenFromCode(const QString & code)
{
    QUrlQuery postData;
    postData.addQueryItem("grant_type", "authorization_code");
    postData.addQueryItem("client_id", clientId);
    postData.addQueryItem("client_secret", clientSecret);
    postData.addQueryItem("redirect_uri", oauth2.replyHandler()->callback());
    postData.addQueryItem("code", code);
    QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(tokenUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = mgr->post(req, postDataEncoded);
    connect(reply, &QNetworkReply::finished, [=](){
        if (reply->error() == QNetworkReply::NoError) {
            // Get tokens from response body
            QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
            extractTokensFromJson(json);

            // Schedule next refresh 5m before expiration time
            int timeout = json["expires_in"].toInt() - 300;
            QTimer::singleShot(timeout * 1000, [&]{
                emit refresh_timeout();
            });

            emit authenticated();
        }
        else {
            qDebug() << "Error in exchangeTokenFromCode: " << reply->errorString();
            qDebug() << reply->readAll();
        }

        reply->deleteLater();
    });
}

void AuthWrapper::on_refresh_timeout()
{
    if (!isAuthenticated())
        return;

    QUrlQuery postData;
    postData.addQueryItem("grant_type", "refresh_token");
    postData.addQueryItem("client_id", clientId);
    postData.addQueryItem("client_secret", clientSecret);
    postData.addQueryItem("refresh_token", refreshToken());
    QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(tokenUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = mgr->post(req, postDataEncoded);
    connect(reply, &QNetworkReply::finished, [=](){
        if (reply->error() == QNetworkReply::NoError) {
            // Get tokens from response body
            QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
            extractTokensFromJson(json);

            // Schedule next refresh 5m before expiration time
            int timeout = json["expires_in"].toInt() - 300;
            QTimer::singleShot(timeout * 1000, [&]{
                emit refresh_timeout();
            });
        }
        else {
            _authenticated = false;

            qDebug() << "Error in on_refresh_timeout: " << reply->errorString();
            qDebug() << reply->readAll();
        }

        reply->deleteLater();
    });
}

void AuthWrapper::putAccessToken(QNetworkRequest &req) const
{
    if (isAuthenticated())
    {
        QString bearer = "Bearer " + accessToken();
        req.setRawHeader(QByteArray("Authorization"), bearer.toUtf8());
    }
}

void AuthWrapper::on_authenticated()
{
    _authenticated = true;
    if (view) {
        view->close();
        view->deleteLater();
    }
}

void AuthWrapper::extractTokensFromJson(QJsonObject & json)
{
    tokens[ID] = json["id_token"].toString();
    tokens[ACCESS] = json["access_token"].toString();
    tokens[REFRESH] = json["refresh_token"].toString();
}

bool AuthWrapper::isAuthenticated() const
{
    return _authenticated;
}

QString AuthWrapper::idToken() const
{
    return QString(tokens[ID]);
}

QString AuthWrapper::accessToken() const
{
    return QString(tokens[ACCESS]);
}

QString AuthWrapper::refreshToken() const
{
    return QString(tokens[REFRESH]);
}
