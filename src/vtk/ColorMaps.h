#ifndef ColorMaps_h
#define ColorMaps_h

#include <functional>
#include <map>
#include <string>
#include <vector>

class vtkLookupTable;

/**
 * Util static class to quickly change a vtkLookupTable color map
 */
class ColorMaps final
{
public:
    ColorMaps() = delete;
    ~ColorMaps() = delete;

    /**
   * @return A list of available Color Maps
   */
    static std::vector<std::string> GetColorMapNames();

    /**
   * Default Color Map
   */
    static const std::string DefaultColorMap;

    /**
   * Change LUT color map
   * @param lut vtkLookupTable
   * @param name Color Map name
   */
    static void SetColorMap(vtkLookupTable *lut,
                            const std::string &name = ColorMaps::DefaultColorMap);

private:
    static const std::map<std::string, std::function<void(vtkLookupTable *)>> LookupTables;

    static void SetInferno(vtkLookupTable *lut);
    static void SetViridis(vtkLookupTable *lut);
    static void SetMagma(vtkLookupTable *lut);
    static void SetPlasma(vtkLookupTable *lut);
    static void SetCividis(vtkLookupTable *lut);
    static void SetGray(vtkLookupTable *lut);
    static void SetDefault(vtkLookupTable *lut);
    static void SetDefaultStep(vtkLookupTable *lut);
    static void SetMinMax(vtkLookupTable *lut);
    static void SetGlow(vtkLookupTable *lut);
    static void SetTemperature(vtkLookupTable *lut);
    static void SetSar(vtkLookupTable *lut);
    static void SetPhysicsContour(vtkLookupTable *lut);
    static void SetEField(vtkLookupTable *lut);
    static void SetRun1(vtkLookupTable *lut);
    static void SetRun2(vtkLookupTable *lut);
    static void SetVolRenGreen(vtkLookupTable *lut);
    static void SetVolRenGlow(vtkLookupTable *lut);
    static void SetVolRenRGB(vtkLookupTable *lut);
    static void SetVolRenTwoLev(vtkLookupTable *lut);
    static void SetTenStep(vtkLookupTable *lut);
    static void SetPureRed(vtkLookupTable *lut);
    static void SetPureGreen(vtkLookupTable *lut);
    static void SetPureBlue(vtkLookupTable *lut);
    static void SetAllRed(vtkLookupTable *lut);
    static void SetAllGreen(vtkLookupTable *lut);
    static void SetAllBlue(vtkLookupTable *lut);
    static void SetAllCyan(vtkLookupTable *lut);
    static void SetAllMagenta(vtkLookupTable *lut);
    static void SetAllYellow(vtkLookupTable *lut);
    static void SetAllWhite(vtkLookupTable *lut);
    static void SetAllBlack(vtkLookupTable *lut);
};

#endif
