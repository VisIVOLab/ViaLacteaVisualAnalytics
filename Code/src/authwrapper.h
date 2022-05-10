#ifndef AUTHWRAPPER_H
#define AUTHWRAPPER_H

#include "vlvaurlschemehandler.h"

#include <QOAuthHttpServerReplyHandler>
#include <QObject>

class QNetworkAccessManager;
class QOAuth2AuthorizationCodeFlow;
class QWebEngineView;

class CustomOAuthReplyHandler : public QOAuthHttpServerReplyHandler
{
    Q_OBJECT
public:
    CustomOAuthReplyHandler(QObject *parent = nullptr) : QOAuthHttpServerReplyHandler(parent) { }

    QString callback() const override { return QString("vlva://callback"); }
};

class AuthWrapper : public QObject
{
    Q_OBJECT
public:
    void grant();
    virtual void logout();
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
    enum Token { ID, ACCESS, REFRESH };

    QUrl authUrl;
    QUrl tokenUrl;
    QUrl logoutUrl;
    QString clientId;
    QString clientSecret;
    QString scope;
    bool _authenticated;
    bool _init;

    QWebEngineView *view;
    VLVAUrlSchemeHandler *handler;
    QNetworkAccessManager *mgr;
    QOAuth2AuthorizationCodeFlow *oauth2;
    QString tokens[3];

    virtual void setup() = 0;
    void extractTokensFromJson(const QJsonObject &json);
    void scheduleRefresh(int timeout);
};

class IA2VlkbAuth : public AuthWrapper
{
    Q_OBJECT
public:
    static AuthWrapper &Instance();
    void logout() override;

private:
    IA2VlkbAuth(QObject *parent = nullptr);

protected:
    void setup() override;
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
