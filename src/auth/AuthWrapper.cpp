#include "AuthWrapper.h"

#include "AuthKeys.h"
#include "OIDCAuthorizationCodeFlow.h"

AuthWrapper::AuthWrapper(QObject *parent) : QObject(parent)
{
    this->vlkb = new OIDCAuthorizationCodeFlow(VLKB_AUTH_URL, VLKB_TOKEN_URL, VLKB_SCOPE,
                                               VLKB_CLIENT_ID, VLKB_SECRET, this);
    QObject::connect(this->vlkb, &OIDCAuthorizationCodeFlow::granted, this,
                     [this]() { emit this->granted(AuthService::VLKB); });
    QObject::connect(this->vlkb, &OIDCAuthorizationCodeFlow::loggedOut, this,
                     [this]() { emit this->loggedOut(AuthService::VLKB); });
}

bool AuthWrapper::hasTokens(AuthService service) const
{
    return this->authObject(service)->hasTokens();
}

void AuthWrapper::putAuthorizationHeader(AuthService service, QNetworkRequest &req)
{
    this->authObject(service)->putAuthorizationHeader(req);
}

void AuthWrapper::grant(AuthService service)
{
    this->authObject(service)->grant();
}

void AuthWrapper::logout(AuthService service)
{
    this->authObject(service)->logout();
}

OIDCAuthorizationCodeFlow *AuthWrapper::authObject(AuthService service) const
{
    return (service == AuthService::VLKB) ? this->vlkb : nullptr;
}
