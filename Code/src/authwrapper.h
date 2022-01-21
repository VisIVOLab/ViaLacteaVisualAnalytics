#ifndef AUTHWRAPPER_H
#define AUTHWRAPPER_H

#include "vlvaurlschemehandler.h"

#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QObject>
#include <QWebEngineView>

class CustomOAuthReplyHandler : public QOAuthHttpServerReplyHandler
{
    Q_OBJECT
public:
    CustomOAuthReplyHandler(QObject *parent = nullptr) : QOAuthHttpServerReplyHandler(parent) { }

    QString callback() const { return QString("vlva://callback"); }
};

class AuthWrapper : public QObject
{
    Q_OBJECT
public:
    void grant();
    void logout();
    bool isAuthenticated() const;
    QString idToken() const;
    QString accessToken() const;
    QString refreshToken() const;
    void putAccessToken(QNetworkRequest &req) const;

signals:
    void authenticated();
    void refresh_timeout();
    void logged_out();

protected slots:
    void open_webview(const QUrl &url);
    void exchange_tokens(const QString &code);
    void on_authenticated();
    void on_refresh_timeout();
    void on_logged_out();

protected:
    AuthWrapper(QObject *parent = nullptr);

    QUrl authUrl;
    QUrl tokenUrl;
    QUrl logoutUrl;
    QString clientId;
    QString clientSecret;
    QString scope;
    bool _authenticated = false;
    bool _init = false;

    QWebEngineView *view;
    VLVAUrlSchemeHandler *handler;
    QNetworkAccessManager *mgr;
    QOAuth2AuthorizationCodeFlow *oauth2;
    enum Token { ID, ACCESS, REFRESH };
    QString tokens[3];

    virtual void setup() = 0;
    void extractTokensFromJson(QJsonObject &json);
};

class NeaniasVlkbAuth : public AuthWrapper
{
    Q_OBJECT
public:
    static AuthWrapper &Instance();

private:
    NeaniasVlkbAuth(QObject *parent = nullptr);

protected:
    void setup() override;
};

class NeaniasCaesarAuth : public AuthWrapper
{
    Q_OBJECT
public:
    static AuthWrapper &Instance();

private:
    NeaniasCaesarAuth(QObject *parent = nullptr);

protected:
    void setup() override;
};

#endif // AUTHWRAPPER_H
