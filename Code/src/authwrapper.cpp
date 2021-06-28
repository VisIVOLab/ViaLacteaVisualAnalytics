#include "authwrapper.h"

#include <QTimer>
#include <QtNetwork>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>

AuthWrapper::AuthWrapper(QObject *parent) : QObject(parent)
{    
    mgr = new QNetworkAccessManager(this);

    // Install vlva:// scheme handler
    handler = new VLVAUrlSchemeHandler(this);
    connect(handler, &VLVAUrlSchemeHandler::codeReceived, this, &AuthWrapper::exchange_tokens);

    connect(this, &AuthWrapper::authenticated, this, &AuthWrapper::on_authenticated);
    connect(this, &AuthWrapper::refresh_timeout, this, &AuthWrapper::on_refresh_timeout);
    connect(this, &AuthWrapper::logged_out, this, &AuthWrapper::on_logged_out);
}

void AuthWrapper::grant()
{
    oauth2->grant();
}

void AuthWrapper::logout()
{
    if (!isAuthenticated())
        return;

    QUrlQuery postData;
    postData.addQueryItem("client_id", clientId);
    postData.addQueryItem("client_secret", clientSecret);
    postData.addQueryItem("refresh_token", refreshToken());
    QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(logoutUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = mgr->post(req, postDataEncoded);
    connect(reply, &QNetworkReply::finished, [=](){
        if (reply->error() == QNetworkReply::NoError) {
            emit logged_out();
        } else {
            qDebug() << "Error in logout: " << reply->errorString();
            qDebug() << reply->readAll();
        }
        reply->deleteLater();
    });
}

void AuthWrapper::on_logged_out()
{
    tokens[ID].clear();
    tokens[ACCESS].clear();
    tokens[REFRESH].clear();
    _authenticated = false;
}

void AuthWrapper::open_webview(const QUrl & url)
{
    // The default UA does not allow standard @gmail.com users to login
    QString userAgent("Mozilla/5.0 (Linux x86_64; rv:87.0) Gecko/20100101 Firefox/87.0");

    view = new QWebEngineView;
    view->page()->profile()->setHttpUserAgent(userAgent);
    view->page()->profile()->installUrlSchemeHandler("vlva", handler);
    view->load(url);
    view->setContextMenuPolicy(Qt::NoContextMenu);
    view->resize(1024, 768);
    view->show();
}

void AuthWrapper::exchange_tokens(const QString & code)
{
    QUrlQuery postData;
    postData.addQueryItem("grant_type", "authorization_code");
    postData.addQueryItem("client_id", clientId);
    postData.addQueryItem("client_secret", clientSecret);
    postData.addQueryItem("redirect_uri", oauth2->replyHandler()->callback());
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

            // Schedule next refresh 2m before expiration time
            int timeout = json["expires_in"].toInt() - 120;
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

            // Schedule next refresh 2m before expiration time
            int timeout = json["expires_in"].toInt() - 120;
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
        view->page()->profile()->cookieStore()->deleteAllCookies();
        view->page()->profile()->removeUrlSchemeHandler(handler);
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

NeaniasVlkbAuth::NeaniasVlkbAuth(QObject *parent): AuthWrapper(parent)
{
    setup();
}

void NeaniasVlkbAuth::setup()
{
    authUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-production/protocol/openid-connect/auth");
    tokenUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-production/protocol/openid-connect/token");
    logoutUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-production/protocol/openid-connect/logout");
    clientId = QString("vlkb");
    clientSecret = QString("b6f98dba-a409-432a-bb21-13b692d86172");
    scope = QString("openid profile email phone address");

    auto replyHandler = new CustomOAuthReplyHandler(this);
    oauth2 = new QOAuth2AuthorizationCodeFlow(this);
    oauth2->setReplyHandler(replyHandler);
    oauth2->setAuthorizationUrl(authUrl);
    oauth2->setAccessTokenUrl(tokenUrl);
    oauth2->setClientIdentifier(clientId);
    oauth2->setScope(scope);
    connect(oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &NeaniasVlkbAuth::open_webview);
}

AuthWrapper& NeaniasVlkbAuth::Instance()
{
    static NeaniasVlkbAuth instance;
    return instance;
}

NeaniasCaesarAuth::NeaniasCaesarAuth(QObject *parent): AuthWrapper(parent)
{
    setup();
}

void NeaniasCaesarAuth::setup()
{
    authUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-development/protocol/openid-connect/auth");
    tokenUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-development/protocol/openid-connect/token");
    logoutUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-development/protocol/openid-connect/logout");
    clientId = QString("caesar-service");
    clientSecret = QString("d06f6533-4d21-4ae8-8fa6-eed159fe02bb");
    scope = QString("openid profile email phone address");

    auto replyHandler = new CustomOAuthReplyHandler(this);
    oauth2 = new QOAuth2AuthorizationCodeFlow(this);
    oauth2->setReplyHandler(replyHandler);
    oauth2->setAuthorizationUrl(authUrl);
    oauth2->setAccessTokenUrl(tokenUrl);
    oauth2->setClientIdentifier(clientId);
    oauth2->setScope(scope);
    connect(oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &NeaniasCaesarAuth::open_webview);
}

AuthWrapper& NeaniasCaesarAuth::Instance()
{
    static NeaniasCaesarAuth instance;
    return instance;
}
