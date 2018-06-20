#ifndef VISIVOFILTERDESKTOP_H
#define VISIVOFILTERDESKTOP_H


class VisIVOFilterDesktop
{
public:
    VisIVOFilterDesktop();
    static void runFilter(int index);
    ~VisIVOFilterDesktop();
private:
    static void addIdentifier();

};

#endif // VISIVOFILTERDESKTOP_H
