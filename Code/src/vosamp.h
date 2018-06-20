#ifndef VOSAMP_H
#define VOSAMP_H


class vosamp
{
public:
    vosamp();
    ~vosamp();
    void unregister();
    void sendFitsImage(char *f, char *t="all");
    char* getClientsList();


};

#endif // VOSAMP_H
