#include "OIDCAuthorizationCodeFlow.h"

#include "HttpServerReplyHandler.h"
#include "Logging.h"
#include "VisIVOUrlSchemeHandler.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOAuth2AuthorizationCodeFlow>
#include <QTimer>
#include <QUrlQuery>
#include <QWebEngineProfile>
#include <QWebEngineView>

using namespace Qt::StringLiterals;

OIDCAuthorizationCodeFlow::OIDCAuthorizationCodeFlow(const QString &authUrl,
                                                     const QString &tokenUrl, const QString &scope,
                                                     const QString &clientId,
                                                     const QString &clientSecret, QObject *parent)
    : QObject(parent)
{
    QSet<QByteArray> scopes;
    for (const auto &s : scope.split(' ')) {
        scopes.insert(s.toUtf8());
    }

    this->nam = new QNetworkAccessManager(this);
    this->auth = new QOAuth2AuthorizationCodeFlow(this->nam, this);
    this->auth->setReplyHandler(new HttpServerReplyHandler(this));
    this->auth->setAuthorizationUrl(QUrl(authUrl));
    this->auth->setTokenUrl(QUrl(tokenUrl));
    this->auth->setRequestedScopeTokens(scopes);
    this->auth->setClientIdentifier(clientId);
    this->auth->setClientIdentifierSharedKey(clientSecret);
    QObject::connect(this->auth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this,
                     &OIDCAuthorizationCodeFlow::authorizeWithWebView);

    this->urlSchemeHandler = new VisIVOUrlSchemeHandler(this);
    QObject::connect(this->urlSchemeHandler, &VisIVOUrlSchemeHandler::callbackReceived, this,
                     &OIDCAuthorizationCodeFlow::authorizationCallbackReceived);

    this->refreshTimer = new QTimer(this);
    QObject::connect(this->refreshTimer, &QTimer::timeout, this,
                     &OIDCAuthorizationCodeFlow::refreshTokens);
}

bool OIDCAuthorizationCodeFlow::hasTokens() const
{
    return !this->accessToken.isEmpty();
}

void OIDCAuthorizationCodeFlow::putAuthorizationHeader(QNetworkRequest &req)
{
    if (this->hasTokens()) {
        const QByteArray value = "Bearer "_ba.append(this->accessToken.toUtf8());
        req.setRawHeader("Authorization"_ba, value);
    }
}

void OIDCAuthorizationCodeFlow::grant()
{
    this->auth->grant();
}

void OIDCAuthorizationCodeFlow::logout()
{
    this->refreshTimer->stop();
    this->accessToken.clear();
    this->refreshToken.clear();
    emit this->loggedOut();
}

void OIDCAuthorizationCodeFlow::authorizeWithWebView(const QUrl &url)
{
    auto profile = new QWebEngineProfile;
    profile->installUrlSchemeHandler("vlva"_ba, this->urlSchemeHandler);

    auto view = new QWebEngineView(profile);
    view->setWindowFlag(Qt::Window);
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->setContextMenuPolicy(Qt::NoContextMenu);
    view->resize(1024, 768);
    view->load(url);
    view->show();
    view->raise();
    view->activateWindow();

    QObject::connect(this->urlSchemeHandler, &VisIVOUrlSchemeHandler::callbackReceived, view,
                     &QWebEngineView::close);
    QObject::connect(view, &QWebEngineView::destroyed, profile, &QWebEngineProfile::deleteLater);
}

void OIDCAuthorizationCodeFlow::authorizationCallbackReceived(const QVariantMap &data)
{
    QByteArray authHeader =
            u"%1:%2"_s.arg(this->auth->clientIdentifier(), this->auth->clientIdentifierSharedKey())
                    .toUtf8()
                    .toBase64();
    authHeader.prepend("Basic "_ba);

    QNetworkRequest req(this->auth->tokenUrl());
    req.setHeader(QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s);
    req.setRawHeader("Authorization"_ba, authHeader);

    QUrlQuery postData;
    postData.addQueryItem(u"grant_type"_s, u"authorization_code"_s);
    postData.addQueryItem(u"client_id"_s, this->auth->clientIdentifier());
    postData.addQueryItem(u"client_secret"_s, this->auth->clientIdentifierSharedKey());
    postData.addQueryItem(u"redirect_uri"_s, this->auth->replyHandler()->callback());
    postData.addQueryItem(u"code"_s, data.value(u"code"_s).toString());
    const QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    auto reply = this->nam->post(req, postDataEncoded);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error()) {
            qCCritical(logApp) << "Could not get user's tokens:" << reply->errorString();
            QMessageBox::critical(nullptr, u"Error"_s, reply->errorString());
            return;
        }

        this->tokensReceived(reply->readAll());
        emit this->granted();
    });
}

void OIDCAuthorizationCodeFlow::tokensReceived(const QByteArray &data)
{
    const QJsonObject response = QJsonDocument::fromJson(data).object();
    this->accessToken = response.value("access_token"_L1).toString();
    this->refreshToken = response.value("refresh_token"_L1).toString();

    const int expiration = response.value("expires_in"_L1).toInt();
    if (expiration > 60) {
        this->refreshTimer->start((expiration - 60) * 1000);
    }
}

void OIDCAuthorizationCodeFlow::refreshTokens()
{
    if (!this->hasTokens()) {
        qCWarning(logApp) << "Trying to refresh user's tokens but the user is no longer "
                             "logged in. Stopping the timer...";
        this->refreshTimer->stop();
        return;
    }

    QByteArray authHeader =
            u"%1:%2"_s.arg(this->auth->clientIdentifier(), this->auth->clientIdentifierSharedKey())
                    .toUtf8()
                    .toBase64();
    authHeader.prepend("Basic "_ba);

    QNetworkRequest req(this->auth->tokenUrl());
    req.setHeader(QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s);
    req.setRawHeader("Authorization"_ba, authHeader);

    QUrlQuery postData;
    postData.addQueryItem(u"grant_type"_s, u"refresh_token"_s);
    postData.addQueryItem(u"client_id"_s, this->auth->clientIdentifier());
    postData.addQueryItem(u"client_secret"_s, this->auth->clientIdentifierSharedKey());
    postData.addQueryItem(u"refresh_token"_s, this->refreshToken);
    const QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    auto reply = this->nam->post(req, postDataEncoded);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error()) {
            this->logout();
            qCCritical(logApp) << "Could not refresh user's tokens:" << reply->errorString();
            QMessageBox::critical(nullptr, u"Error"_s, reply->errorString());
            return;
        }

        this->tokensReceived(reply->readAll());
    });
}
