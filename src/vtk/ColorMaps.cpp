#include "ColorMaps.h"

#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>

using namespace std::string_literals;

const std::map<std::string, std::function<void(vtkLookupTable *)>> ColorMaps::LookupTables = {
    { "Inferno"s, &ColorMaps::SetInferno },
    { "Viridis"s, &ColorMaps::SetViridis },
    { "Magma"s, &ColorMaps::SetMagma },
    { "Plasma"s, &ColorMaps::SetPlasma },
    { "Cividis"s, &ColorMaps::SetCividis },
    { "Gray"s, &ColorMaps::SetGray },
    { "Default"s, &ColorMaps::SetDefault },
    { "DefaultStep"s, &ColorMaps::SetDefaultStep },
    { "MinMax"s, &ColorMaps::SetMinMax },
    { "Glow"s, &ColorMaps::SetGlow },
    { "Temperature"s, &ColorMaps::SetTemperature },
    { "Sar"s, &ColorMaps::SetSar },
    { "PhysicsContour"s, &ColorMaps::SetPhysicsContour },
    { "EField"s, &ColorMaps::SetEField },
    { "Run1"s, &ColorMaps::SetRun1 },
    { "Run2"s, &ColorMaps::SetRun2 },
    { "VolRenGreen"s, &ColorMaps::SetVolRenGreen },
    { "VolRenGlow"s, &ColorMaps::SetVolRenGlow },
    { "VolRenRGB"s, &ColorMaps::SetVolRenRGB },
    { "VolRenTwoLev"s, &ColorMaps::SetVolRenTwoLev },
    { "TenStep"s, &ColorMaps::SetTenStep },
    { "PureRed"s, &ColorMaps::SetPureRed },
    { "PureGreen"s, &ColorMaps::SetPureGreen },
    { "PureBlue"s, &ColorMaps::SetPureBlue },
    { "AllRed"s, &ColorMaps::SetAllRed },
    { "AllGreen"s, &ColorMaps::SetAllGreen },
    { "AllBlue"s, &ColorMaps::SetAllBlue },
    { "AllCyan"s, &ColorMaps::SetAllCyan },
    { "AllMagenta"s, &ColorMaps::SetAllMagenta },
    { "AllYellow"s, &ColorMaps::SetAllYellow },
    { "AllWhite"s, &ColorMaps::SetAllWhite },
    { "AllBlack"s, &ColorMaps::SetAllBlack }
};

const std::string ColorMaps::DefaultColorMap = "Inferno"s;

void ColorMaps::SetColorMap(vtkLookupTable *lut, const std::string &name)
{
    if (lut->GetObjectName() == name) {
        return;
    }

    auto cmap = ColorMaps::LookupTables.find(name);
    if (cmap != ColorMaps::LookupTables.cend()) {
        cmap->second(lut);
        lut->SetObjectName(name);
    }
}

void ColorMaps::SetColorTransferFunction(vtkLookupTable *lut, vtkColorTransferFunction *ctf)
{
    ctf->RemoveAllPoints();

    double range[2];
    lut->GetTableRange(range);

    const vtkIdType n = lut->GetNumberOfTableValues();
    for (vtkIdType i = 0; i < n; ++i) {
        const double t = static_cast<double>(i) / (n - 1);
        const double value = range[0] + t * (range[1] - range[0]);
        double rgba[4];
        lut->GetTableValue(i, rgba);
        ctf->AddRGBPoint(value, rgba[0], rgba[1], rgba[2]);
    }
}

std::vector<std::string> ColorMaps::GetColorMapNames()
{
    std::vector<std::string> names;
    names.reserve(ColorMaps::LookupTables.size());
    std::transform(ColorMaps::LookupTables.cbegin(), ColorMaps::LookupTables.cend(),
                   std::back_inserter(names), [](const auto &cmap) { return cmap.first; });
    return names;
}

void ColorMaps::SetInferno(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    lut->SetTableValue(0, 0.001462, 0.000466, 0.013866);
    lut->SetTableValue(1, 0.002267, 0.00127, 0.01857);
    lut->SetTableValue(2, 0.003299, 0.002249, 0.024239);
    lut->SetTableValue(3, 0.004547, 0.003392, 0.030909);
    lut->SetTableValue(4, 0.006006, 0.004692, 0.038558);
    lut->SetTableValue(5, 0.007676, 0.006136, 0.046836);
    lut->SetTableValue(6, 0.009561, 0.007713, 0.055143);
    lut->SetTableValue(7, 0.011663, 0.009417, 0.06346);
    lut->SetTableValue(8, 0.013995, 0.011225, 0.071862);
    lut->SetTableValue(9, 0.016561, 0.013136, 0.080282);
    lut->SetTableValue(10, 0.019373, 0.015133, 0.088767);
    lut->SetTableValue(11, 0.022447, 0.017199, 0.097327);
    lut->SetTableValue(12, 0.025793, 0.019331, 0.10593);
    lut->SetTableValue(13, 0.029432, 0.021503, 0.114621);
    lut->SetTableValue(14, 0.033385, 0.023702, 0.123397);
    lut->SetTableValue(15, 0.037668, 0.025921, 0.132232);
    lut->SetTableValue(16, 0.042253, 0.028139, 0.141141);
    lut->SetTableValue(17, 0.046915, 0.030324, 0.150164);
    lut->SetTableValue(18, 0.051644, 0.032474, 0.159254);
    lut->SetTableValue(19, 0.056449, 0.034569, 0.168414);
    lut->SetTableValue(20, 0.06134, 0.03659, 0.177642);
    lut->SetTableValue(21, 0.066331, 0.038504, 0.186962);
    lut->SetTableValue(22, 0.071429, 0.040294, 0.196354);
    lut->SetTableValue(23, 0.076637, 0.041905, 0.205799);
    lut->SetTableValue(24, 0.081962, 0.043328, 0.215289);
    lut->SetTableValue(25, 0.087411, 0.044556, 0.224813);
    lut->SetTableValue(26, 0.09299, 0.045583, 0.234358);
    lut->SetTableValue(27, 0.098702, 0.046402, 0.243904);
    lut->SetTableValue(28, 0.104551, 0.047008, 0.25343);
    lut->SetTableValue(29, 0.110536, 0.047399, 0.262912);
    lut->SetTableValue(30, 0.116656, 0.047574, 0.272321);
    lut->SetTableValue(31, 0.122908, 0.047536, 0.281624);
    lut->SetTableValue(32, 0.129285, 0.047293, 0.290788);
    lut->SetTableValue(33, 0.135778, 0.046856, 0.299776);
    lut->SetTableValue(34, 0.142378, 0.046242, 0.308553);
    lut->SetTableValue(35, 0.149073, 0.045468, 0.317085);
    lut->SetTableValue(36, 0.15585, 0.044559, 0.325338);
    lut->SetTableValue(37, 0.162689, 0.043554, 0.333277);
    lut->SetTableValue(38, 0.169575, 0.042489, 0.340874);
    lut->SetTableValue(39, 0.176493, 0.041402, 0.348111);
    lut->SetTableValue(40, 0.183429, 0.040329, 0.354971);
    lut->SetTableValue(41, 0.190367, 0.039309, 0.361447);
    lut->SetTableValue(42, 0.197297, 0.0384, 0.367535);
    lut->SetTableValue(43, 0.204209, 0.037632, 0.373238);
    lut->SetTableValue(44, 0.211095, 0.03703, 0.378563);
    lut->SetTableValue(45, 0.217949, 0.036615, 0.383522);
    lut->SetTableValue(46, 0.224763, 0.036405, 0.388129);
    lut->SetTableValue(47, 0.231538, 0.036405, 0.3924);
    lut->SetTableValue(48, 0.238273, 0.036621, 0.396353);
    lut->SetTableValue(49, 0.244967, 0.037055, 0.400007);
    lut->SetTableValue(50, 0.25162, 0.037705, 0.403378);
    lut->SetTableValue(51, 0.258234, 0.038571, 0.406485);
    lut->SetTableValue(52, 0.26481, 0.039647, 0.409345);
    lut->SetTableValue(53, 0.271347, 0.040922, 0.411976);
    lut->SetTableValue(54, 0.27785, 0.042353, 0.414392);
    lut->SetTableValue(55, 0.284321, 0.043933, 0.416608);
    lut->SetTableValue(56, 0.290763, 0.045644, 0.418637);
    lut->SetTableValue(57, 0.297178, 0.04747, 0.420491);
    lut->SetTableValue(58, 0.303568, 0.049396, 0.422182);
    lut->SetTableValue(59, 0.309935, 0.051407, 0.423721);
    lut->SetTableValue(60, 0.316282, 0.05349, 0.425116);
    lut->SetTableValue(61, 0.32261, 0.055634, 0.426377);
    lut->SetTableValue(62, 0.328921, 0.057827, 0.427511);
    lut->SetTableValue(63, 0.335217, 0.06006, 0.428524);
    lut->SetTableValue(64, 0.3415, 0.062325, 0.429425);
    lut->SetTableValue(65, 0.347771, 0.064616, 0.430217);
    lut->SetTableValue(66, 0.354032, 0.066925, 0.430906);
    lut->SetTableValue(67, 0.360284, 0.069247, 0.431497);
    lut->SetTableValue(68, 0.366529, 0.071579, 0.431994);
    lut->SetTableValue(69, 0.372768, 0.073915, 0.4324);
    lut->SetTableValue(70, 0.379001, 0.076253, 0.432719);
    lut->SetTableValue(71, 0.385228, 0.078591, 0.432955);
    lut->SetTableValue(72, 0.391453, 0.080927, 0.433109);
    lut->SetTableValue(73, 0.397674, 0.083257, 0.433183);
    lut->SetTableValue(74, 0.403894, 0.08558, 0.433179);
    lut->SetTableValue(75, 0.410113, 0.087896, 0.433098);
    lut->SetTableValue(76, 0.416331, 0.090203, 0.432943);
    lut->SetTableValue(77, 0.422549, 0.092501, 0.432714);
    lut->SetTableValue(78, 0.428768, 0.09479, 0.432412);
    lut->SetTableValue(79, 0.434987, 0.097069, 0.432039);
    lut->SetTableValue(80, 0.441207, 0.099338, 0.431594);
    lut->SetTableValue(81, 0.447428, 0.101597, 0.43108);
    lut->SetTableValue(82, 0.453651, 0.103848, 0.430498);
    lut->SetTableValue(83, 0.459875, 0.106089, 0.429846);
    lut->SetTableValue(84, 0.4661, 0.108322, 0.429125);
    lut->SetTableValue(85, 0.472328, 0.110547, 0.428334);
    lut->SetTableValue(86, 0.478558, 0.112764, 0.427475);
    lut->SetTableValue(87, 0.484789, 0.114974, 0.426548);
    lut->SetTableValue(88, 0.491022, 0.117179, 0.425552);
    lut->SetTableValue(89, 0.497257, 0.119379, 0.424488);
    lut->SetTableValue(90, 0.503493, 0.121575, 0.423356);
    lut->SetTableValue(91, 0.50973, 0.123769, 0.422156);
    lut->SetTableValue(92, 0.515967, 0.12596, 0.420887);
    lut->SetTableValue(93, 0.522206, 0.12815, 0.419549);
    lut->SetTableValue(94, 0.528444, 0.130341, 0.418142);
    lut->SetTableValue(95, 0.534683, 0.132534, 0.416667);
    lut->SetTableValue(96, 0.54092, 0.134729, 0.415123);
    lut->SetTableValue(97, 0.547157, 0.136929, 0.413511);
    lut->SetTableValue(98, 0.553392, 0.139134, 0.411829);
    lut->SetTableValue(99, 0.559624, 0.141346, 0.410078);
    lut->SetTableValue(100, 0.565854, 0.143567, 0.408258);
    lut->SetTableValue(101, 0.572081, 0.145797, 0.406369);
    lut->SetTableValue(102, 0.578304, 0.148039, 0.404411);
    lut->SetTableValue(103, 0.584521, 0.150294, 0.402385);
    lut->SetTableValue(104, 0.590734, 0.152563, 0.40029);
    lut->SetTableValue(105, 0.59694, 0.154848, 0.398125);
    lut->SetTableValue(106, 0.603139, 0.157151, 0.395891);
    lut->SetTableValue(107, 0.60933, 0.159474, 0.393589);
    lut->SetTableValue(108, 0.615513, 0.161817, 0.391219);
    lut->SetTableValue(109, 0.621685, 0.164184, 0.388781);
    lut->SetTableValue(110, 0.627847, 0.166575, 0.386276);
    lut->SetTableValue(111, 0.633998, 0.168992, 0.383704);
    lut->SetTableValue(112, 0.640135, 0.171438, 0.381065);
    lut->SetTableValue(113, 0.64626, 0.173914, 0.378359);
    lut->SetTableValue(114, 0.652369, 0.176421, 0.375586);
    lut->SetTableValue(115, 0.658463, 0.178962, 0.372748);
    lut->SetTableValue(116, 0.66454, 0.181539, 0.369846);
    lut->SetTableValue(117, 0.670599, 0.184153, 0.366879);
    lut->SetTableValue(118, 0.676638, 0.186807, 0.363849);
    lut->SetTableValue(119, 0.682656, 0.189501, 0.360757);
    lut->SetTableValue(120, 0.688653, 0.192239, 0.357603);
    lut->SetTableValue(121, 0.694627, 0.195021, 0.354388);
    lut->SetTableValue(122, 0.700576, 0.197851, 0.351113);
    lut->SetTableValue(123, 0.7065, 0.200728, 0.347777);
    lut->SetTableValue(124, 0.712396, 0.203656, 0.344383);
    lut->SetTableValue(125, 0.718264, 0.206636, 0.340931);
    lut->SetTableValue(126, 0.724103, 0.20967, 0.337424);
    lut->SetTableValue(127, 0.729909, 0.212759, 0.333861);
    lut->SetTableValue(128, 0.735683, 0.215906, 0.330245);
    lut->SetTableValue(129, 0.741423, 0.219112, 0.326576);
    lut->SetTableValue(130, 0.747127, 0.222378, 0.322856);
    lut->SetTableValue(131, 0.752794, 0.225706, 0.319085);
    lut->SetTableValue(132, 0.758422, 0.229097, 0.315266);
    lut->SetTableValue(133, 0.76401, 0.232554, 0.311399);
    lut->SetTableValue(134, 0.769556, 0.236077, 0.307485);
    lut->SetTableValue(135, 0.775059, 0.239667, 0.303526);
    lut->SetTableValue(136, 0.780517, 0.243327, 0.299523);
    lut->SetTableValue(137, 0.785929, 0.247056, 0.295477);
    lut->SetTableValue(138, 0.791293, 0.250856, 0.29139);
    lut->SetTableValue(139, 0.796607, 0.254728, 0.287264);
    lut->SetTableValue(140, 0.801871, 0.258674, 0.283099);
    lut->SetTableValue(141, 0.807082, 0.262692, 0.278898);
    lut->SetTableValue(142, 0.812239, 0.266786, 0.274661);
    lut->SetTableValue(143, 0.817341, 0.270954, 0.27039);
    lut->SetTableValue(144, 0.822386, 0.275197, 0.266085);
    lut->SetTableValue(145, 0.827372, 0.279517, 0.26175);
    lut->SetTableValue(146, 0.832299, 0.283913, 0.257383);
    lut->SetTableValue(147, 0.837165, 0.288385, 0.252988);
    lut->SetTableValue(148, 0.841969, 0.292933, 0.248564);
    lut->SetTableValue(149, 0.846709, 0.297559, 0.244113);
    lut->SetTableValue(150, 0.851384, 0.30226, 0.239636);
    lut->SetTableValue(151, 0.855992, 0.307038, 0.235133);
    lut->SetTableValue(152, 0.860533, 0.311892, 0.230606);
    lut->SetTableValue(153, 0.865006, 0.316822, 0.226055);
    lut->SetTableValue(154, 0.869409, 0.321827, 0.221482);
    lut->SetTableValue(155, 0.873741, 0.326906, 0.216886);
    lut->SetTableValue(156, 0.878001, 0.33206, 0.212268);
    lut->SetTableValue(157, 0.882188, 0.337287, 0.207628);
    lut->SetTableValue(158, 0.886302, 0.342586, 0.202968);
    lut->SetTableValue(159, 0.890341, 0.347957, 0.198286);
    lut->SetTableValue(160, 0.894305, 0.353399, 0.193584);
    lut->SetTableValue(161, 0.898192, 0.358911, 0.18886);
    lut->SetTableValue(162, 0.902003, 0.364492, 0.184116);
    lut->SetTableValue(163, 0.905735, 0.37014, 0.17935);
    lut->SetTableValue(164, 0.90939, 0.375856, 0.174563);
    lut->SetTableValue(165, 0.912966, 0.381636, 0.169755);
    lut->SetTableValue(166, 0.916462, 0.387481, 0.164924);
    lut->SetTableValue(167, 0.919879, 0.393389, 0.16007);
    lut->SetTableValue(168, 0.923215, 0.399359, 0.155193);
    lut->SetTableValue(169, 0.92647, 0.405389, 0.150292);
    lut->SetTableValue(170, 0.929644, 0.411479, 0.145367);
    lut->SetTableValue(171, 0.932737, 0.417627, 0.140417);
    lut->SetTableValue(172, 0.935747, 0.423831, 0.13544);
    lut->SetTableValue(173, 0.938675, 0.430091, 0.130438);
    lut->SetTableValue(174, 0.941521, 0.436405, 0.125409);
    lut->SetTableValue(175, 0.944285, 0.442772, 0.120354);
    lut->SetTableValue(176, 0.946965, 0.449191, 0.115272);
    lut->SetTableValue(177, 0.949562, 0.45566, 0.110164);
    lut->SetTableValue(178, 0.952075, 0.462178, 0.105031);
    lut->SetTableValue(179, 0.954506, 0.468744, 0.099874);
    lut->SetTableValue(180, 0.956852, 0.475356, 0.094695);
    lut->SetTableValue(181, 0.959114, 0.482014, 0.089499);
    lut->SetTableValue(182, 0.961293, 0.488716, 0.084289);
    lut->SetTableValue(183, 0.963387, 0.495462, 0.079073);
    lut->SetTableValue(184, 0.965397, 0.502249, 0.073859);
    lut->SetTableValue(185, 0.967322, 0.509078, 0.068659);
    lut->SetTableValue(186, 0.969163, 0.515946, 0.063488);
    lut->SetTableValue(187, 0.970919, 0.522853, 0.058367);
    lut->SetTableValue(188, 0.97259, 0.529798, 0.053324);
    lut->SetTableValue(189, 0.974176, 0.53678, 0.048392);
    lut->SetTableValue(190, 0.975677, 0.543798, 0.043618);
    lut->SetTableValue(191, 0.977092, 0.55085, 0.03905);
    lut->SetTableValue(192, 0.978422, 0.557937, 0.034931);
    lut->SetTableValue(193, 0.979666, 0.565057, 0.031409);
    lut->SetTableValue(194, 0.980824, 0.572209, 0.028508);
    lut->SetTableValue(195, 0.981895, 0.579392, 0.02625);
    lut->SetTableValue(196, 0.982881, 0.586606, 0.024661);
    lut->SetTableValue(197, 0.983779, 0.593849, 0.02377);
    lut->SetTableValue(198, 0.984591, 0.601122, 0.023606);
    lut->SetTableValue(199, 0.985315, 0.608422, 0.024202);
    lut->SetTableValue(200, 0.985952, 0.61575, 0.025592);
    lut->SetTableValue(201, 0.986502, 0.623105, 0.027814);
    lut->SetTableValue(202, 0.986964, 0.630485, 0.030908);
    lut->SetTableValue(203, 0.987337, 0.63789, 0.034916);
    lut->SetTableValue(204, 0.987622, 0.64532, 0.039886);
    lut->SetTableValue(205, 0.987819, 0.652773, 0.045581);
    lut->SetTableValue(206, 0.987926, 0.66025, 0.05175);
    lut->SetTableValue(207, 0.987945, 0.667748, 0.058329);
    lut->SetTableValue(208, 0.987874, 0.675267, 0.065257);
    lut->SetTableValue(209, 0.987714, 0.682807, 0.072489);
    lut->SetTableValue(210, 0.987464, 0.690366, 0.07999);
    lut->SetTableValue(211, 0.987124, 0.697944, 0.087731);
    lut->SetTableValue(212, 0.986694, 0.70554, 0.095694);
    lut->SetTableValue(213, 0.986175, 0.713153, 0.103863);
    lut->SetTableValue(214, 0.985566, 0.720782, 0.112229);
    lut->SetTableValue(215, 0.984865, 0.728427, 0.120785);
    lut->SetTableValue(216, 0.984075, 0.736087, 0.129527);
    lut->SetTableValue(217, 0.983196, 0.743758, 0.138453);
    lut->SetTableValue(218, 0.982228, 0.751442, 0.147565);
    lut->SetTableValue(219, 0.981173, 0.759135, 0.156863);
    lut->SetTableValue(220, 0.980032, 0.766837, 0.166353);
    lut->SetTableValue(221, 0.978806, 0.774545, 0.176037);
    lut->SetTableValue(222, 0.977497, 0.782258, 0.185923);
    lut->SetTableValue(223, 0.976108, 0.789974, 0.196018);
    lut->SetTableValue(224, 0.974638, 0.797692, 0.206332);
    lut->SetTableValue(225, 0.973088, 0.805409, 0.216877);
    lut->SetTableValue(226, 0.971468, 0.813122, 0.227658);
    lut->SetTableValue(227, 0.969783, 0.820825, 0.238686);
    lut->SetTableValue(228, 0.968041, 0.828515, 0.249972);
    lut->SetTableValue(229, 0.966243, 0.836191, 0.261534);
    lut->SetTableValue(230, 0.964394, 0.843848, 0.273391);
    lut->SetTableValue(231, 0.962517, 0.851476, 0.285546);
    lut->SetTableValue(232, 0.960626, 0.859069, 0.29801);
    lut->SetTableValue(233, 0.95872, 0.866624, 0.31082);
    lut->SetTableValue(234, 0.956834, 0.874129, 0.323974);
    lut->SetTableValue(235, 0.954997, 0.881569, 0.337475);
    lut->SetTableValue(236, 0.953215, 0.888942, 0.351369);
    lut->SetTableValue(237, 0.951546, 0.896226, 0.365627);
    lut->SetTableValue(238, 0.950018, 0.903409, 0.380271);
    lut->SetTableValue(239, 0.948683, 0.910473, 0.395289);
    lut->SetTableValue(240, 0.947594, 0.917399, 0.410665);
    lut->SetTableValue(241, 0.946809, 0.924168, 0.426373);
    lut->SetTableValue(242, 0.946392, 0.930761, 0.442367);
    lut->SetTableValue(243, 0.946403, 0.937159, 0.458592);
    lut->SetTableValue(244, 0.946903, 0.943348, 0.47497);
    lut->SetTableValue(245, 0.947937, 0.949318, 0.491426);
    lut->SetTableValue(246, 0.949545, 0.955063, 0.50786);
    lut->SetTableValue(247, 0.95174, 0.960587, 0.524203);
    lut->SetTableValue(248, 0.954529, 0.965896, 0.540361);
    lut->SetTableValue(249, 0.957896, 0.971003, 0.556275);
    lut->SetTableValue(250, 0.961812, 0.975924, 0.571925);
    lut->SetTableValue(251, 0.966249, 0.980678, 0.587206);
    lut->SetTableValue(252, 0.971162, 0.985282, 0.602154);
    lut->SetTableValue(253, 0.976511, 0.989753, 0.61676);
    lut->SetTableValue(254, 0.982257, 0.994109, 0.631017);
    lut->SetTableValue(255, 0.988362, 0.998364, 0.644924);
}

void ColorMaps::SetViridis(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    lut->SetTableValue(0, 0.267004, 0.004874, 0.329415);
    lut->SetTableValue(1, 0.26851, 0.009605, 0.335427);
    lut->SetTableValue(2, 0.269944, 0.014625, 0.341379);
    lut->SetTableValue(3, 0.271305, 0.019942, 0.347269);
    lut->SetTableValue(4, 0.272594, 0.025563, 0.353093);
    lut->SetTableValue(5, 0.273809, 0.031497, 0.358853);
    lut->SetTableValue(6, 0.274952, 0.037752, 0.364543);
    lut->SetTableValue(7, 0.276022, 0.044167, 0.370164);
    lut->SetTableValue(8, 0.277018, 0.050344, 0.375715);
    lut->SetTableValue(9, 0.277941, 0.056324, 0.381191);
    lut->SetTableValue(10, 0.278791, 0.062145, 0.386592);
    lut->SetTableValue(11, 0.279566, 0.067836, 0.391917);
    lut->SetTableValue(12, 0.280267, 0.073417, 0.397163);
    lut->SetTableValue(13, 0.280894, 0.078907, 0.402329);
    lut->SetTableValue(14, 0.281446, 0.08432, 0.407414);
    lut->SetTableValue(15, 0.281924, 0.089666, 0.412415);
    lut->SetTableValue(16, 0.282327, 0.094955, 0.417331);
    lut->SetTableValue(17, 0.282656, 0.100196, 0.42216);
    lut->SetTableValue(18, 0.28291, 0.105393, 0.426902);
    lut->SetTableValue(19, 0.283091, 0.110553, 0.431554);
    lut->SetTableValue(20, 0.283197, 0.11568, 0.436115);
    lut->SetTableValue(21, 0.283229, 0.120777, 0.440584);
    lut->SetTableValue(22, 0.283187, 0.125848, 0.44496);
    lut->SetTableValue(23, 0.283072, 0.130895, 0.449241);
    lut->SetTableValue(24, 0.282884, 0.13592, 0.453427);
    lut->SetTableValue(25, 0.282623, 0.140926, 0.457517);
    lut->SetTableValue(26, 0.28229, 0.145912, 0.46151);
    lut->SetTableValue(27, 0.281887, 0.150881, 0.465405);
    lut->SetTableValue(28, 0.281412, 0.155834, 0.469201);
    lut->SetTableValue(29, 0.280868, 0.160771, 0.472899);
    lut->SetTableValue(30, 0.280255, 0.165693, 0.476498);
    lut->SetTableValue(31, 0.279574, 0.170599, 0.479997);
    lut->SetTableValue(32, 0.278826, 0.17549, 0.483397);
    lut->SetTableValue(33, 0.278012, 0.180367, 0.486697);
    lut->SetTableValue(34, 0.277134, 0.185228, 0.489898);
    lut->SetTableValue(35, 0.276194, 0.190074, 0.493001);
    lut->SetTableValue(36, 0.275191, 0.194905, 0.496005);
    lut->SetTableValue(37, 0.274128, 0.199721, 0.498911);
    lut->SetTableValue(38, 0.273006, 0.20452, 0.501721);
    lut->SetTableValue(39, 0.271828, 0.209303, 0.504434);
    lut->SetTableValue(40, 0.270595, 0.214069, 0.507052);
    lut->SetTableValue(41, 0.269308, 0.218818, 0.509577);
    lut->SetTableValue(42, 0.267968, 0.223549, 0.512008);
    lut->SetTableValue(43, 0.26658, 0.228262, 0.514349);
    lut->SetTableValue(44, 0.265145, 0.232956, 0.516599);
    lut->SetTableValue(45, 0.263663, 0.237631, 0.518762);
    lut->SetTableValue(46, 0.262138, 0.242286, 0.520837);
    lut->SetTableValue(47, 0.260571, 0.246922, 0.522828);
    lut->SetTableValue(48, 0.258965, 0.251537, 0.524736);
    lut->SetTableValue(49, 0.257322, 0.25613, 0.526563);
    lut->SetTableValue(50, 0.255645, 0.260703, 0.528312);
    lut->SetTableValue(51, 0.253935, 0.265254, 0.529983);
    lut->SetTableValue(52, 0.252194, 0.269783, 0.531579);
    lut->SetTableValue(53, 0.250425, 0.27429, 0.533103);
    lut->SetTableValue(54, 0.248629, 0.278775, 0.534556);
    lut->SetTableValue(55, 0.246811, 0.283237, 0.535941);
    lut->SetTableValue(56, 0.244972, 0.287675, 0.53726);
    lut->SetTableValue(57, 0.243113, 0.292092, 0.538516);
    lut->SetTableValue(58, 0.241237, 0.296485, 0.539709);
    lut->SetTableValue(59, 0.239346, 0.300855, 0.540844);
    lut->SetTableValue(60, 0.237441, 0.305202, 0.541921);
    lut->SetTableValue(61, 0.235526, 0.309527, 0.542944);
    lut->SetTableValue(62, 0.233603, 0.313828, 0.543914);
    lut->SetTableValue(63, 0.231674, 0.318106, 0.544834);
    lut->SetTableValue(64, 0.229739, 0.322361, 0.545706);
    lut->SetTableValue(65, 0.227802, 0.326594, 0.546532);
    lut->SetTableValue(66, 0.225863, 0.330805, 0.547314);
    lut->SetTableValue(67, 0.223925, 0.334994, 0.548053);
    lut->SetTableValue(68, 0.221989, 0.339161, 0.548752);
    lut->SetTableValue(69, 0.220057, 0.343307, 0.549413);
    lut->SetTableValue(70, 0.21813, 0.347432, 0.550038);
    lut->SetTableValue(71, 0.21621, 0.351535, 0.550627);
    lut->SetTableValue(72, 0.214298, 0.355619, 0.551184);
    lut->SetTableValue(73, 0.212395, 0.359683, 0.55171);
    lut->SetTableValue(74, 0.210503, 0.363727, 0.552206);
    lut->SetTableValue(75, 0.208623, 0.367752, 0.552675);
    lut->SetTableValue(76, 0.206756, 0.371758, 0.553117);
    lut->SetTableValue(77, 0.204903, 0.375746, 0.553533);
    lut->SetTableValue(78, 0.203063, 0.379716, 0.553925);
    lut->SetTableValue(79, 0.201239, 0.38367, 0.554294);
    lut->SetTableValue(80, 0.19943, 0.387607, 0.554642);
    lut->SetTableValue(81, 0.197636, 0.391528, 0.554969);
    lut->SetTableValue(82, 0.19586, 0.395433, 0.555276);
    lut->SetTableValue(83, 0.1941, 0.399323, 0.555565);
    lut->SetTableValue(84, 0.192357, 0.403199, 0.555836);
    lut->SetTableValue(85, 0.190631, 0.407061, 0.556089);
    lut->SetTableValue(86, 0.188923, 0.41091, 0.556326);
    lut->SetTableValue(87, 0.187231, 0.414746, 0.556547);
    lut->SetTableValue(88, 0.185556, 0.41857, 0.556753);
    lut->SetTableValue(89, 0.183898, 0.422383, 0.556944);
    lut->SetTableValue(90, 0.182256, 0.426184, 0.55712);
    lut->SetTableValue(91, 0.180629, 0.429975, 0.557282);
    lut->SetTableValue(92, 0.179019, 0.433756, 0.55743);
    lut->SetTableValue(93, 0.177423, 0.437527, 0.557565);
    lut->SetTableValue(94, 0.175841, 0.44129, 0.557685);
    lut->SetTableValue(95, 0.174274, 0.445044, 0.557792);
    lut->SetTableValue(96, 0.172719, 0.448791, 0.557885);
    lut->SetTableValue(97, 0.171176, 0.45253, 0.557965);
    lut->SetTableValue(98, 0.169646, 0.456262, 0.55803);
    lut->SetTableValue(99, 0.168126, 0.459988, 0.558082);
    lut->SetTableValue(100, 0.166617, 0.463708, 0.558119);
    lut->SetTableValue(101, 0.165117, 0.467423, 0.558141);
    lut->SetTableValue(102, 0.163625, 0.471133, 0.558148);
    lut->SetTableValue(103, 0.162142, 0.474838, 0.55814);
    lut->SetTableValue(104, 0.160665, 0.47854, 0.558115);
    lut->SetTableValue(105, 0.159194, 0.482237, 0.558073);
    lut->SetTableValue(106, 0.157729, 0.485932, 0.558013);
    lut->SetTableValue(107, 0.15627, 0.489624, 0.557936);
    lut->SetTableValue(108, 0.154815, 0.493313, 0.55784);
    lut->SetTableValue(109, 0.153364, 0.497, 0.557724);
    lut->SetTableValue(110, 0.151918, 0.500685, 0.557587);
    lut->SetTableValue(111, 0.150476, 0.504369, 0.55743);
    lut->SetTableValue(112, 0.149039, 0.508051, 0.55725);
    lut->SetTableValue(113, 0.147607, 0.511733, 0.557049);
    lut->SetTableValue(114, 0.14618, 0.515413, 0.556823);
    lut->SetTableValue(115, 0.144759, 0.519093, 0.556572);
    lut->SetTableValue(116, 0.143343, 0.522773, 0.556295);
    lut->SetTableValue(117, 0.141935, 0.526453, 0.555991);
    lut->SetTableValue(118, 0.140536, 0.530132, 0.555659);
    lut->SetTableValue(119, 0.139147, 0.533812, 0.555298);
    lut->SetTableValue(120, 0.13777, 0.537492, 0.554906);
    lut->SetTableValue(121, 0.136408, 0.541173, 0.554483);
    lut->SetTableValue(122, 0.135066, 0.544853, 0.554029);
    lut->SetTableValue(123, 0.133743, 0.548535, 0.553541);
    lut->SetTableValue(124, 0.132444, 0.552216, 0.553018);
    lut->SetTableValue(125, 0.131172, 0.555899, 0.552459);
    lut->SetTableValue(126, 0.129933, 0.559582, 0.551864);
    lut->SetTableValue(127, 0.128729, 0.563265, 0.551229);
    lut->SetTableValue(128, 0.127568, 0.566949, 0.550556);
    lut->SetTableValue(129, 0.126453, 0.570633, 0.549841);
    lut->SetTableValue(130, 0.125394, 0.574318, 0.549086);
    lut->SetTableValue(131, 0.124395, 0.578002, 0.548287);
    lut->SetTableValue(132, 0.123463, 0.581687, 0.547445);
    lut->SetTableValue(133, 0.122606, 0.585371, 0.546557);
    lut->SetTableValue(134, 0.121831, 0.589055, 0.545623);
    lut->SetTableValue(135, 0.121148, 0.592739, 0.544641);
    lut->SetTableValue(136, 0.120565, 0.596422, 0.543611);
    lut->SetTableValue(137, 0.120092, 0.600104, 0.54253);
    lut->SetTableValue(138, 0.119738, 0.603785, 0.5414);
    lut->SetTableValue(139, 0.119512, 0.607464, 0.540218);
    lut->SetTableValue(140, 0.119423, 0.611141, 0.538982);
    lut->SetTableValue(141, 0.119483, 0.614817, 0.537692);
    lut->SetTableValue(142, 0.119699, 0.61849, 0.536347);
    lut->SetTableValue(143, 0.120081, 0.622161, 0.534946);
    lut->SetTableValue(144, 0.120638, 0.625828, 0.533488);
    lut->SetTableValue(145, 0.12138, 0.629492, 0.531973);
    lut->SetTableValue(146, 0.122312, 0.633153, 0.530398);
    lut->SetTableValue(147, 0.123444, 0.636809, 0.528763);
    lut->SetTableValue(148, 0.12478, 0.640461, 0.527068);
    lut->SetTableValue(149, 0.126326, 0.644107, 0.525311);
    lut->SetTableValue(150, 0.128087, 0.647749, 0.523491);
    lut->SetTableValue(151, 0.130067, 0.651384, 0.521608);
    lut->SetTableValue(152, 0.132268, 0.655014, 0.519661);
    lut->SetTableValue(153, 0.134692, 0.658636, 0.517649);
    lut->SetTableValue(154, 0.137339, 0.662252, 0.515571);
    lut->SetTableValue(155, 0.14021, 0.665859, 0.513427);
    lut->SetTableValue(156, 0.143303, 0.669459, 0.511215);
    lut->SetTableValue(157, 0.146616, 0.67305, 0.508936);
    lut->SetTableValue(158, 0.150148, 0.676631, 0.506589);
    lut->SetTableValue(159, 0.153894, 0.680203, 0.504172);
    lut->SetTableValue(160, 0.157851, 0.683765, 0.501686);
    lut->SetTableValue(161, 0.162016, 0.687316, 0.499129);
    lut->SetTableValue(162, 0.166383, 0.690856, 0.496502);
    lut->SetTableValue(163, 0.170948, 0.694384, 0.493803);
    lut->SetTableValue(164, 0.175707, 0.6979, 0.491033);
    lut->SetTableValue(165, 0.180653, 0.701402, 0.488189);
    lut->SetTableValue(166, 0.185783, 0.704891, 0.485273);
    lut->SetTableValue(167, 0.19109, 0.708366, 0.482284);
    lut->SetTableValue(168, 0.196571, 0.711827, 0.479221);
    lut->SetTableValue(169, 0.202219, 0.715272, 0.476084);
    lut->SetTableValue(170, 0.20803, 0.718701, 0.472873);
    lut->SetTableValue(171, 0.214, 0.722114, 0.469588);
    lut->SetTableValue(172, 0.220124, 0.725509, 0.466226);
    lut->SetTableValue(173, 0.226397, 0.728888, 0.462789);
    lut->SetTableValue(174, 0.232815, 0.732247, 0.459277);
    lut->SetTableValue(175, 0.239374, 0.735588, 0.455688);
    lut->SetTableValue(176, 0.24607, 0.73891, 0.452024);
    lut->SetTableValue(177, 0.252899, 0.742211, 0.448284);
    lut->SetTableValue(178, 0.259857, 0.745492, 0.444467);
    lut->SetTableValue(179, 0.266941, 0.748751, 0.440573);
    lut->SetTableValue(180, 0.274149, 0.751988, 0.436601);
    lut->SetTableValue(181, 0.281477, 0.755203, 0.432552);
    lut->SetTableValue(182, 0.288921, 0.758394, 0.428426);
    lut->SetTableValue(183, 0.296479, 0.761561, 0.424223);
    lut->SetTableValue(184, 0.304148, 0.764704, 0.419943);
    lut->SetTableValue(185, 0.311925, 0.767822, 0.415586);
    lut->SetTableValue(186, 0.319809, 0.770914, 0.411152);
    lut->SetTableValue(187, 0.327796, 0.77398, 0.40664);
    lut->SetTableValue(188, 0.335885, 0.777018, 0.402049);
    lut->SetTableValue(189, 0.344074, 0.780029, 0.397381);
    lut->SetTableValue(190, 0.35236, 0.783011, 0.392636);
    lut->SetTableValue(191, 0.360741, 0.785964, 0.387814);
    lut->SetTableValue(192, 0.369214, 0.788888, 0.382914);
    lut->SetTableValue(193, 0.377779, 0.791781, 0.377939);
    lut->SetTableValue(194, 0.386433, 0.794644, 0.372886);
    lut->SetTableValue(195, 0.395174, 0.797475, 0.367757);
    lut->SetTableValue(196, 0.404001, 0.800275, 0.362552);
    lut->SetTableValue(197, 0.412913, 0.803041, 0.357269);
    lut->SetTableValue(198, 0.421908, 0.805774, 0.35191);
    lut->SetTableValue(199, 0.430983, 0.808473, 0.346476);
    lut->SetTableValue(200, 0.440137, 0.811138, 0.340967);
    lut->SetTableValue(201, 0.449368, 0.813768, 0.335384);
    lut->SetTableValue(202, 0.458674, 0.816363, 0.329727);
    lut->SetTableValue(203, 0.468053, 0.818921, 0.323998);
    lut->SetTableValue(204, 0.477504, 0.821444, 0.318195);
    lut->SetTableValue(205, 0.487026, 0.823929, 0.312321);
    lut->SetTableValue(206, 0.496615, 0.826376, 0.306377);
    lut->SetTableValue(207, 0.506271, 0.828786, 0.300362);
    lut->SetTableValue(208, 0.515992, 0.831158, 0.294279);
    lut->SetTableValue(209, 0.525776, 0.833491, 0.288127);
    lut->SetTableValue(210, 0.535621, 0.835785, 0.281908);
    lut->SetTableValue(211, 0.545524, 0.838039, 0.275626);
    lut->SetTableValue(212, 0.555484, 0.840254, 0.269281);
    lut->SetTableValue(213, 0.565498, 0.84243, 0.262877);
    lut->SetTableValue(214, 0.575563, 0.844566, 0.256415);
    lut->SetTableValue(215, 0.585678, 0.846661, 0.249897);
    lut->SetTableValue(216, 0.595839, 0.848717, 0.243329);
    lut->SetTableValue(217, 0.606045, 0.850733, 0.236712);
    lut->SetTableValue(218, 0.616293, 0.852709, 0.230052);
    lut->SetTableValue(219, 0.626579, 0.854645, 0.223353);
    lut->SetTableValue(220, 0.636902, 0.856542, 0.21662);
    lut->SetTableValue(221, 0.647257, 0.8584, 0.209861);
    lut->SetTableValue(222, 0.657642, 0.860219, 0.203082);
    lut->SetTableValue(223, 0.668054, 0.861999, 0.196293);
    lut->SetTableValue(224, 0.678489, 0.863742, 0.189503);
    lut->SetTableValue(225, 0.688944, 0.865448, 0.182725);
    lut->SetTableValue(226, 0.699415, 0.867117, 0.175971);
    lut->SetTableValue(227, 0.709898, 0.868751, 0.169257);
    lut->SetTableValue(228, 0.720391, 0.87035, 0.162603);
    lut->SetTableValue(229, 0.730889, 0.871916, 0.156029);
    lut->SetTableValue(230, 0.741388, 0.873449, 0.149561);
    lut->SetTableValue(231, 0.751884, 0.874951, 0.143228);
    lut->SetTableValue(232, 0.762373, 0.876424, 0.137064);
    lut->SetTableValue(233, 0.772852, 0.877868, 0.131109);
    lut->SetTableValue(234, 0.783315, 0.879285, 0.125405);
    lut->SetTableValue(235, 0.79376, 0.880678, 0.120005);
    lut->SetTableValue(236, 0.804182, 0.882046, 0.114965);
    lut->SetTableValue(237, 0.814576, 0.883393, 0.110347);
    lut->SetTableValue(238, 0.82494, 0.88472, 0.106217);
    lut->SetTableValue(239, 0.83527, 0.886029, 0.102646);
    lut->SetTableValue(240, 0.845561, 0.887322, 0.099702);
    lut->SetTableValue(241, 0.85581, 0.888601, 0.097452);
    lut->SetTableValue(242, 0.866013, 0.889868, 0.095953);
    lut->SetTableValue(243, 0.876168, 0.891125, 0.09525);
    lut->SetTableValue(244, 0.886271, 0.892374, 0.095374);
    lut->SetTableValue(245, 0.89632, 0.893616, 0.096335);
    lut->SetTableValue(246, 0.906311, 0.894855, 0.098125);
    lut->SetTableValue(247, 0.916242, 0.896091, 0.100717);
    lut->SetTableValue(248, 0.926106, 0.89733, 0.104071);
    lut->SetTableValue(249, 0.935904, 0.89857, 0.108131);
    lut->SetTableValue(250, 0.945636, 0.899815, 0.112838);
    lut->SetTableValue(251, 0.9553, 0.901065, 0.118128);
    lut->SetTableValue(252, 0.964894, 0.902323, 0.123941);
    lut->SetTableValue(253, 0.974417, 0.90359, 0.130215);
    lut->SetTableValue(254, 0.983868, 0.904867, 0.136897);
    lut->SetTableValue(255, 0.993248, 0.906157, 0.143936);
}

void ColorMaps::SetMagma(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    lut->SetTableValue(0, 0.001462, 0.000466, 0.013866);
    lut->SetTableValue(1, 0.002258, 0.001295, 0.018331);
    lut->SetTableValue(2, 0.003279, 0.002305, 0.023708);
    lut->SetTableValue(3, 0.004512, 0.00349, 0.029965);
    lut->SetTableValue(4, 0.00595, 0.004843, 0.03713);
    lut->SetTableValue(5, 0.007588, 0.006356, 0.044973);
    lut->SetTableValue(6, 0.009426, 0.008022, 0.052844);
    lut->SetTableValue(7, 0.011465, 0.009828, 0.06075);
    lut->SetTableValue(8, 0.013708, 0.011771, 0.068667);
    lut->SetTableValue(9, 0.016156, 0.01384, 0.076603);
    lut->SetTableValue(10, 0.018815, 0.016026, 0.084584);
    lut->SetTableValue(11, 0.021692, 0.01832, 0.09261);
    lut->SetTableValue(12, 0.024792, 0.020715, 0.100676);
    lut->SetTableValue(13, 0.028123, 0.023201, 0.108787);
    lut->SetTableValue(14, 0.031696, 0.025765, 0.116965);
    lut->SetTableValue(15, 0.03552, 0.028397, 0.125209);
    lut->SetTableValue(16, 0.039608, 0.03109, 0.133515);
    lut->SetTableValue(17, 0.04383, 0.03383, 0.141886);
    lut->SetTableValue(18, 0.048062, 0.036607, 0.150327);
    lut->SetTableValue(19, 0.05232, 0.039407, 0.158841);
    lut->SetTableValue(20, 0.056615, 0.04216, 0.167446);
    lut->SetTableValue(21, 0.060949, 0.044794, 0.176129);
    lut->SetTableValue(22, 0.06533, 0.047318, 0.184892);
    lut->SetTableValue(23, 0.069764, 0.049726, 0.193735);
    lut->SetTableValue(24, 0.074257, 0.052017, 0.20266);
    lut->SetTableValue(25, 0.078815, 0.054184, 0.211667);
    lut->SetTableValue(26, 0.083446, 0.056225, 0.220755);
    lut->SetTableValue(27, 0.088155, 0.058133, 0.229922);
    lut->SetTableValue(28, 0.092949, 0.059904, 0.239164);
    lut->SetTableValue(29, 0.097833, 0.061531, 0.248477);
    lut->SetTableValue(30, 0.102815, 0.06301, 0.257854);
    lut->SetTableValue(31, 0.107899, 0.064335, 0.267289);
    lut->SetTableValue(32, 0.113094, 0.065492, 0.276784);
    lut->SetTableValue(33, 0.118405, 0.066479, 0.286321);
    lut->SetTableValue(34, 0.123833, 0.067295, 0.295879);
    lut->SetTableValue(35, 0.12938, 0.067935, 0.305443);
    lut->SetTableValue(36, 0.135053, 0.068391, 0.315);
    lut->SetTableValue(37, 0.140858, 0.068654, 0.324538);
    lut->SetTableValue(38, 0.146785, 0.068738, 0.334011);
    lut->SetTableValue(39, 0.152839, 0.068637, 0.343404);
    lut->SetTableValue(40, 0.159018, 0.068354, 0.352688);
    lut->SetTableValue(41, 0.165308, 0.067911, 0.361816);
    lut->SetTableValue(42, 0.171713, 0.067305, 0.370771);
    lut->SetTableValue(43, 0.178212, 0.066576, 0.379497);
    lut->SetTableValue(44, 0.184801, 0.065732, 0.387973);
    lut->SetTableValue(45, 0.19146, 0.064818, 0.396152);
    lut->SetTableValue(46, 0.198177, 0.063862, 0.404009);
    lut->SetTableValue(47, 0.204935, 0.062907, 0.411514);
    lut->SetTableValue(48, 0.211718, 0.061992, 0.418647);
    lut->SetTableValue(49, 0.218512, 0.061158, 0.425392);
    lut->SetTableValue(50, 0.225302, 0.060445, 0.431742);
    lut->SetTableValue(51, 0.232077, 0.059889, 0.437695);
    lut->SetTableValue(52, 0.238826, 0.059517, 0.443256);
    lut->SetTableValue(53, 0.245543, 0.059352, 0.448436);
    lut->SetTableValue(54, 0.25222, 0.059415, 0.453248);
    lut->SetTableValue(55, 0.258857, 0.059706, 0.45771);
    lut->SetTableValue(56, 0.265447, 0.060237, 0.46184);
    lut->SetTableValue(57, 0.271994, 0.060994, 0.46566);
    lut->SetTableValue(58, 0.278493, 0.061978, 0.46919);
    lut->SetTableValue(59, 0.284951, 0.063168, 0.472451);
    lut->SetTableValue(60, 0.291366, 0.064553, 0.475462);
    lut->SetTableValue(61, 0.29774, 0.066117, 0.478243);
    lut->SetTableValue(62, 0.304081, 0.067835, 0.480812);
    lut->SetTableValue(63, 0.310382, 0.069702, 0.483186);
    lut->SetTableValue(64, 0.316654, 0.07169, 0.48538);
    lut->SetTableValue(65, 0.322899, 0.073782, 0.487408);
    lut->SetTableValue(66, 0.329114, 0.075972, 0.489287);
    lut->SetTableValue(67, 0.335308, 0.078236, 0.491024);
    lut->SetTableValue(68, 0.341482, 0.080564, 0.492631);
    lut->SetTableValue(69, 0.347636, 0.082946, 0.494121);
    lut->SetTableValue(70, 0.353773, 0.085373, 0.495501);
    lut->SetTableValue(71, 0.359898, 0.087831, 0.496778);
    lut->SetTableValue(72, 0.366012, 0.090314, 0.49796);
    lut->SetTableValue(73, 0.372116, 0.092816, 0.499053);
    lut->SetTableValue(74, 0.378211, 0.095332, 0.500067);
    lut->SetTableValue(75, 0.384299, 0.097855, 0.501002);
    lut->SetTableValue(76, 0.390384, 0.100379, 0.501864);
    lut->SetTableValue(77, 0.396467, 0.102902, 0.502658);
    lut->SetTableValue(78, 0.402548, 0.10542, 0.503386);
    lut->SetTableValue(79, 0.408629, 0.10793, 0.504052);
    lut->SetTableValue(80, 0.414709, 0.110431, 0.504662);
    lut->SetTableValue(81, 0.420791, 0.11292, 0.505215);
    lut->SetTableValue(82, 0.426877, 0.115395, 0.505714);
    lut->SetTableValue(83, 0.432967, 0.117855, 0.50616);
    lut->SetTableValue(84, 0.439062, 0.120298, 0.506555);
    lut->SetTableValue(85, 0.445163, 0.122724, 0.506901);
    lut->SetTableValue(86, 0.451271, 0.125132, 0.507198);
    lut->SetTableValue(87, 0.457386, 0.127522, 0.507448);
    lut->SetTableValue(88, 0.463508, 0.129893, 0.507652);
    lut->SetTableValue(89, 0.46964, 0.132245, 0.507809);
    lut->SetTableValue(90, 0.47578, 0.134577, 0.507921);
    lut->SetTableValue(91, 0.481929, 0.136891, 0.507989);
    lut->SetTableValue(92, 0.488088, 0.139186, 0.508011);
    lut->SetTableValue(93, 0.494258, 0.141462, 0.507988);
    lut->SetTableValue(94, 0.500438, 0.143719, 0.50792);
    lut->SetTableValue(95, 0.506629, 0.145958, 0.507806);
    lut->SetTableValue(96, 0.512831, 0.148179, 0.507648);
    lut->SetTableValue(97, 0.519045, 0.150383, 0.507443);
    lut->SetTableValue(98, 0.52527, 0.152569, 0.507192);
    lut->SetTableValue(99, 0.531507, 0.154739, 0.506895);
    lut->SetTableValue(100, 0.537755, 0.156894, 0.506551);
    lut->SetTableValue(101, 0.544015, 0.159033, 0.506159);
    lut->SetTableValue(102, 0.550287, 0.161158, 0.505719);
    lut->SetTableValue(103, 0.556571, 0.163269, 0.50523);
    lut->SetTableValue(104, 0.562866, 0.165368, 0.504692);
    lut->SetTableValue(105, 0.569172, 0.167454, 0.504105);
    lut->SetTableValue(106, 0.57549, 0.16953, 0.503466);
    lut->SetTableValue(107, 0.581819, 0.171596, 0.502777);
    lut->SetTableValue(108, 0.588158, 0.173652, 0.502035);
    lut->SetTableValue(109, 0.594508, 0.175701, 0.501241);
    lut->SetTableValue(110, 0.600868, 0.177743, 0.500394);
    lut->SetTableValue(111, 0.607238, 0.179779, 0.499492);
    lut->SetTableValue(112, 0.613617, 0.181811, 0.498536);
    lut->SetTableValue(113, 0.620005, 0.18384, 0.497524);
    lut->SetTableValue(114, 0.626401, 0.185867, 0.496456);
    lut->SetTableValue(115, 0.632805, 0.187893, 0.495332);
    lut->SetTableValue(116, 0.639216, 0.189921, 0.49415);
    lut->SetTableValue(117, 0.645633, 0.191952, 0.49291);
    lut->SetTableValue(118, 0.652056, 0.193986, 0.491611);
    lut->SetTableValue(119, 0.658483, 0.196027, 0.490253);
    lut->SetTableValue(120, 0.664915, 0.198075, 0.488836);
    lut->SetTableValue(121, 0.671349, 0.200133, 0.487358);
    lut->SetTableValue(122, 0.677786, 0.202203, 0.485819);
    lut->SetTableValue(123, 0.684224, 0.204286, 0.484219);
    lut->SetTableValue(124, 0.690661, 0.206384, 0.482558);
    lut->SetTableValue(125, 0.697098, 0.208501, 0.480835);
    lut->SetTableValue(126, 0.703532, 0.210638, 0.479049);
    lut->SetTableValue(127, 0.709962, 0.212797, 0.477201);
    lut->SetTableValue(128, 0.716387, 0.214982, 0.47529);
    lut->SetTableValue(129, 0.722805, 0.217194, 0.473316);
    lut->SetTableValue(130, 0.729216, 0.219437, 0.471279);
    lut->SetTableValue(131, 0.735616, 0.221713, 0.46918);
    lut->SetTableValue(132, 0.742004, 0.224025, 0.467018);
    lut->SetTableValue(133, 0.748378, 0.226377, 0.464794);
    lut->SetTableValue(134, 0.754737, 0.228772, 0.462509);
    lut->SetTableValue(135, 0.761077, 0.231214, 0.460162);
    lut->SetTableValue(136, 0.767398, 0.233705, 0.457755);
    lut->SetTableValue(137, 0.773695, 0.236249, 0.455289);
    lut->SetTableValue(138, 0.779968, 0.238851, 0.452765);
    lut->SetTableValue(139, 0.786212, 0.241514, 0.450184);
    lut->SetTableValue(140, 0.792427, 0.244242, 0.447543);
    lut->SetTableValue(141, 0.798608, 0.24704, 0.444848);
    lut->SetTableValue(142, 0.804752, 0.249911, 0.442102);
    lut->SetTableValue(143, 0.810855, 0.252861, 0.439305);
    lut->SetTableValue(144, 0.816914, 0.255895, 0.436461);
    lut->SetTableValue(145, 0.822926, 0.259016, 0.433573);
    lut->SetTableValue(146, 0.828886, 0.262229, 0.430644);
    lut->SetTableValue(147, 0.834791, 0.26554, 0.427671);
    lut->SetTableValue(148, 0.840636, 0.268953, 0.424666);
    lut->SetTableValue(149, 0.846416, 0.272473, 0.421631);
    lut->SetTableValue(150, 0.852126, 0.276106, 0.418573);
    lut->SetTableValue(151, 0.857763, 0.279857, 0.415496);
    lut->SetTableValue(152, 0.86332, 0.283729, 0.412403);
    lut->SetTableValue(153, 0.868793, 0.287728, 0.409303);
    lut->SetTableValue(154, 0.874176, 0.291859, 0.406205);
    lut->SetTableValue(155, 0.879464, 0.296125, 0.403118);
    lut->SetTableValue(156, 0.884651, 0.30053, 0.400047);
    lut->SetTableValue(157, 0.889731, 0.305079, 0.397002);
    lut->SetTableValue(158, 0.8947, 0.309773, 0.393995);
    lut->SetTableValue(159, 0.899552, 0.314616, 0.391037);
    lut->SetTableValue(160, 0.904281, 0.31961, 0.388137);
    lut->SetTableValue(161, 0.908884, 0.324755, 0.385308);
    lut->SetTableValue(162, 0.913354, 0.330052, 0.382563);
    lut->SetTableValue(163, 0.917689, 0.3355, 0.379915);
    lut->SetTableValue(164, 0.921884, 0.341098, 0.377376);
    lut->SetTableValue(165, 0.925937, 0.346844, 0.374959);
    lut->SetTableValue(166, 0.929845, 0.352734, 0.372677);
    lut->SetTableValue(167, 0.933606, 0.358764, 0.370541);
    lut->SetTableValue(168, 0.937221, 0.364929, 0.368567);
    lut->SetTableValue(169, 0.940687, 0.371224, 0.366762);
    lut->SetTableValue(170, 0.944006, 0.377643, 0.365136);
    lut->SetTableValue(171, 0.94718, 0.384178, 0.363701);
    lut->SetTableValue(172, 0.95021, 0.39082, 0.362468);
    lut->SetTableValue(173, 0.953099, 0.397563, 0.361438);
    lut->SetTableValue(174, 0.955849, 0.4044, 0.360619);
    lut->SetTableValue(175, 0.958464, 0.411324, 0.360014);
    lut->SetTableValue(176, 0.960949, 0.418323, 0.35963);
    lut->SetTableValue(177, 0.96331, 0.42539, 0.359469);
    lut->SetTableValue(178, 0.965549, 0.432519, 0.359529);
    lut->SetTableValue(179, 0.967671, 0.439703, 0.35981);
    lut->SetTableValue(180, 0.96968, 0.446936, 0.360311);
    lut->SetTableValue(181, 0.971582, 0.45421, 0.36103);
    lut->SetTableValue(182, 0.973381, 0.46152, 0.361965);
    lut->SetTableValue(183, 0.975082, 0.468861, 0.363111);
    lut->SetTableValue(184, 0.97669, 0.476226, 0.364466);
    lut->SetTableValue(185, 0.97821, 0.483612, 0.366025);
    lut->SetTableValue(186, 0.979645, 0.491014, 0.367783);
    lut->SetTableValue(187, 0.981, 0.498428, 0.369734);
    lut->SetTableValue(188, 0.982279, 0.505851, 0.371874);
    lut->SetTableValue(189, 0.983485, 0.51328, 0.374198);
    lut->SetTableValue(190, 0.984622, 0.520713, 0.376698);
    lut->SetTableValue(191, 0.985693, 0.528148, 0.379371);
    lut->SetTableValue(192, 0.9867, 0.535582, 0.38221);
    lut->SetTableValue(193, 0.987646, 0.543015, 0.38521);
    lut->SetTableValue(194, 0.988533, 0.550446, 0.388365);
    lut->SetTableValue(195, 0.989363, 0.557873, 0.391671);
    lut->SetTableValue(196, 0.990138, 0.565296, 0.395122);
    lut->SetTableValue(197, 0.990871, 0.572706, 0.398714);
    lut->SetTableValue(198, 0.991558, 0.580107, 0.402441);
    lut->SetTableValue(199, 0.992196, 0.587502, 0.406299);
    lut->SetTableValue(200, 0.992785, 0.594891, 0.410283);
    lut->SetTableValue(201, 0.993326, 0.602275, 0.41439);
    lut->SetTableValue(202, 0.993834, 0.609644, 0.418613);
    lut->SetTableValue(203, 0.994309, 0.616999, 0.42295);
    lut->SetTableValue(204, 0.994738, 0.62435, 0.427397);
    lut->SetTableValue(205, 0.995122, 0.631696, 0.431951);
    lut->SetTableValue(206, 0.99548, 0.639027, 0.436607);
    lut->SetTableValue(207, 0.99581, 0.646344, 0.441361);
    lut->SetTableValue(208, 0.996096, 0.653659, 0.446213);
    lut->SetTableValue(209, 0.996341, 0.660969, 0.45116);
    lut->SetTableValue(210, 0.99658, 0.668256, 0.456192);
    lut->SetTableValue(211, 0.996775, 0.675541, 0.461314);
    lut->SetTableValue(212, 0.996925, 0.682828, 0.466526);
    lut->SetTableValue(213, 0.997077, 0.690088, 0.471811);
    lut->SetTableValue(214, 0.997186, 0.697349, 0.477182);
    lut->SetTableValue(215, 0.997254, 0.704611, 0.482635);
    lut->SetTableValue(216, 0.997325, 0.711848, 0.488154);
    lut->SetTableValue(217, 0.997351, 0.719089, 0.493755);
    lut->SetTableValue(218, 0.997351, 0.726324, 0.499428);
    lut->SetTableValue(219, 0.997341, 0.733545, 0.505167);
    lut->SetTableValue(220, 0.997285, 0.740772, 0.510983);
    lut->SetTableValue(221, 0.997228, 0.747981, 0.516859);
    lut->SetTableValue(222, 0.997138, 0.75519, 0.522806);
    lut->SetTableValue(223, 0.997019, 0.762398, 0.528821);
    lut->SetTableValue(224, 0.996898, 0.769591, 0.534892);
    lut->SetTableValue(225, 0.996727, 0.776795, 0.541039);
    lut->SetTableValue(226, 0.996571, 0.783977, 0.547233);
    lut->SetTableValue(227, 0.996369, 0.791167, 0.553499);
    lut->SetTableValue(228, 0.996162, 0.798348, 0.55982);
    lut->SetTableValue(229, 0.995932, 0.805527, 0.566202);
    lut->SetTableValue(230, 0.99568, 0.812706, 0.572645);
    lut->SetTableValue(231, 0.995424, 0.819875, 0.57914);
    lut->SetTableValue(232, 0.995131, 0.827052, 0.585701);
    lut->SetTableValue(233, 0.994851, 0.834213, 0.592307);
    lut->SetTableValue(234, 0.994524, 0.841387, 0.598983);
    lut->SetTableValue(235, 0.994222, 0.84854, 0.605696);
    lut->SetTableValue(236, 0.993866, 0.855711, 0.612482);
    lut->SetTableValue(237, 0.993545, 0.862859, 0.619299);
    lut->SetTableValue(238, 0.99317, 0.870024, 0.626189);
    lut->SetTableValue(239, 0.992831, 0.877168, 0.633109);
    lut->SetTableValue(240, 0.99244, 0.88433, 0.640099);
    lut->SetTableValue(241, 0.992089, 0.89147, 0.647116);
    lut->SetTableValue(242, 0.991688, 0.898627, 0.654202);
    lut->SetTableValue(243, 0.991332, 0.905763, 0.661309);
    lut->SetTableValue(244, 0.99093, 0.912915, 0.668481);
    lut->SetTableValue(245, 0.99057, 0.920049, 0.675675);
    lut->SetTableValue(246, 0.990175, 0.927196, 0.682926);
    lut->SetTableValue(247, 0.989815, 0.934329, 0.690198);
    lut->SetTableValue(248, 0.989434, 0.94147, 0.697519);
    lut->SetTableValue(249, 0.989077, 0.948604, 0.704863);
    lut->SetTableValue(250, 0.988717, 0.955742, 0.712242);
    lut->SetTableValue(251, 0.988367, 0.962878, 0.719649);
    lut->SetTableValue(252, 0.988033, 0.970012, 0.727077);
    lut->SetTableValue(253, 0.987691, 0.977154, 0.734536);
    lut->SetTableValue(254, 0.987387, 0.984288, 0.742002);
    lut->SetTableValue(255, 0.987053, 0.991438, 0.749504);
}

void ColorMaps::SetPlasma(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    lut->SetTableValue(0, 0.050383, 0.029803, 0.527975);
    lut->SetTableValue(1, 0.063536, 0.028426, 0.533124);
    lut->SetTableValue(2, 0.075353, 0.027206, 0.538007);
    lut->SetTableValue(3, 0.086222, 0.026125, 0.542658);
    lut->SetTableValue(4, 0.096379, 0.025165, 0.547103);
    lut->SetTableValue(5, 0.10598, 0.024309, 0.551368);
    lut->SetTableValue(6, 0.115124, 0.023556, 0.555468);
    lut->SetTableValue(7, 0.123903, 0.022878, 0.559423);
    lut->SetTableValue(8, 0.132381, 0.022258, 0.56325);
    lut->SetTableValue(9, 0.140603, 0.021687, 0.566959);
    lut->SetTableValue(10, 0.148607, 0.021154, 0.570562);
    lut->SetTableValue(11, 0.156421, 0.020651, 0.574065);
    lut->SetTableValue(12, 0.16407, 0.020171, 0.577478);
    lut->SetTableValue(13, 0.171574, 0.019706, 0.580806);
    lut->SetTableValue(14, 0.17895, 0.019252, 0.584054);
    lut->SetTableValue(15, 0.186213, 0.018803, 0.587228);
    lut->SetTableValue(16, 0.193374, 0.018354, 0.59033);
    lut->SetTableValue(17, 0.200445, 0.017902, 0.593364);
    lut->SetTableValue(18, 0.207435, 0.017442, 0.596333);
    lut->SetTableValue(19, 0.21435, 0.016973, 0.599239);
    lut->SetTableValue(20, 0.221197, 0.016497, 0.602083);
    lut->SetTableValue(21, 0.227983, 0.016007, 0.604867);
    lut->SetTableValue(22, 0.234715, 0.015502, 0.607592);
    lut->SetTableValue(23, 0.241396, 0.014979, 0.610259);
    lut->SetTableValue(24, 0.248032, 0.014439, 0.612868);
    lut->SetTableValue(25, 0.254627, 0.013882, 0.615419);
    lut->SetTableValue(26, 0.261183, 0.013308, 0.617911);
    lut->SetTableValue(27, 0.267703, 0.012716, 0.620346);
    lut->SetTableValue(28, 0.274191, 0.012109, 0.622722);
    lut->SetTableValue(29, 0.280648, 0.011488, 0.625038);
    lut->SetTableValue(30, 0.287076, 0.010855, 0.627295);
    lut->SetTableValue(31, 0.293478, 0.010213, 0.62949);
    lut->SetTableValue(32, 0.299855, 0.009561, 0.631624);
    lut->SetTableValue(33, 0.30621, 0.008902, 0.633694);
    lut->SetTableValue(34, 0.312543, 0.008239, 0.6357);
    lut->SetTableValue(35, 0.318856, 0.007576, 0.63764);
    lut->SetTableValue(36, 0.32515, 0.006915, 0.639512);
    lut->SetTableValue(37, 0.331426, 0.006261, 0.641316);
    lut->SetTableValue(38, 0.337683, 0.005618, 0.643049);
    lut->SetTableValue(39, 0.343925, 0.004991, 0.64471);
    lut->SetTableValue(40, 0.35015, 0.004382, 0.646298);
    lut->SetTableValue(41, 0.356359, 0.003798, 0.64781);
    lut->SetTableValue(42, 0.362553, 0.003243, 0.649245);
    lut->SetTableValue(43, 0.368733, 0.002724, 0.650601);
    lut->SetTableValue(44, 0.374897, 0.002245, 0.651876);
    lut->SetTableValue(45, 0.381047, 0.001814, 0.653068);
    lut->SetTableValue(46, 0.387183, 0.001434, 0.654177);
    lut->SetTableValue(47, 0.393304, 0.001114, 0.655199);
    lut->SetTableValue(48, 0.399411, 0.000859, 0.656133);
    lut->SetTableValue(49, 0.405503, 0.000678, 0.656977);
    lut->SetTableValue(50, 0.41158, 0.000577, 0.65773);
    lut->SetTableValue(51, 0.417642, 0.000564, 0.65839);
    lut->SetTableValue(52, 0.423689, 0.000646, 0.658956);
    lut->SetTableValue(53, 0.429719, 0.000831, 0.659425);
    lut->SetTableValue(54, 0.435734, 0.001127, 0.659797);
    lut->SetTableValue(55, 0.441732, 0.00154, 0.660069);
    lut->SetTableValue(56, 0.447714, 0.00208, 0.66024);
    lut->SetTableValue(57, 0.453677, 0.002755, 0.66031);
    lut->SetTableValue(58, 0.459623, 0.003574, 0.660277);
    lut->SetTableValue(59, 0.46555, 0.004545, 0.660139);
    lut->SetTableValue(60, 0.471457, 0.005678, 0.659897);
    lut->SetTableValue(61, 0.477344, 0.00698, 0.659549);
    lut->SetTableValue(62, 0.48321, 0.00846, 0.659095);
    lut->SetTableValue(63, 0.489055, 0.010127, 0.658534);
    lut->SetTableValue(64, 0.494877, 0.01199, 0.657865);
    lut->SetTableValue(65, 0.500678, 0.014055, 0.657088);
    lut->SetTableValue(66, 0.506454, 0.016333, 0.656202);
    lut->SetTableValue(67, 0.512206, 0.018833, 0.655209);
    lut->SetTableValue(68, 0.517933, 0.021563, 0.654109);
    lut->SetTableValue(69, 0.523633, 0.024532, 0.652901);
    lut->SetTableValue(70, 0.529306, 0.027747, 0.651586);
    lut->SetTableValue(71, 0.534952, 0.031217, 0.650165);
    lut->SetTableValue(72, 0.54057, 0.03495, 0.64864);
    lut->SetTableValue(73, 0.546157, 0.038954, 0.64701);
    lut->SetTableValue(74, 0.551715, 0.043136, 0.645277);
    lut->SetTableValue(75, 0.557243, 0.047331, 0.643443);
    lut->SetTableValue(76, 0.562738, 0.051545, 0.641509);
    lut->SetTableValue(77, 0.568201, 0.055778, 0.639477);
    lut->SetTableValue(78, 0.573632, 0.060028, 0.637349);
    lut->SetTableValue(79, 0.579029, 0.064296, 0.635126);
    lut->SetTableValue(80, 0.584391, 0.068579, 0.632812);
    lut->SetTableValue(81, 0.589719, 0.072878, 0.630408);
    lut->SetTableValue(82, 0.595011, 0.07719, 0.627917);
    lut->SetTableValue(83, 0.600266, 0.081516, 0.625342);
    lut->SetTableValue(84, 0.605485, 0.085854, 0.622686);
    lut->SetTableValue(85, 0.610667, 0.090204, 0.619951);
    lut->SetTableValue(86, 0.615812, 0.094564, 0.61714);
    lut->SetTableValue(87, 0.620919, 0.098934, 0.614257);
    lut->SetTableValue(88, 0.625987, 0.103312, 0.611305);
    lut->SetTableValue(89, 0.631017, 0.107699, 0.608287);
    lut->SetTableValue(90, 0.636008, 0.112092, 0.605205);
    lut->SetTableValue(91, 0.640959, 0.116492, 0.602065);
    lut->SetTableValue(92, 0.645872, 0.120898, 0.598867);
    lut->SetTableValue(93, 0.650746, 0.125309, 0.595617);
    lut->SetTableValue(94, 0.65558, 0.129725, 0.592317);
    lut->SetTableValue(95, 0.660374, 0.134144, 0.588971);
    lut->SetTableValue(96, 0.665129, 0.138566, 0.585582);
    lut->SetTableValue(97, 0.669845, 0.142992, 0.582154);
    lut->SetTableValue(98, 0.674522, 0.147419, 0.578688);
    lut->SetTableValue(99, 0.67916, 0.151848, 0.575189);
    lut->SetTableValue(100, 0.683758, 0.156278, 0.57166);
    lut->SetTableValue(101, 0.688318, 0.160709, 0.568103);
    lut->SetTableValue(102, 0.69284, 0.165141, 0.564522);
    lut->SetTableValue(103, 0.697324, 0.169573, 0.560919);
    lut->SetTableValue(104, 0.701769, 0.174005, 0.557296);
    lut->SetTableValue(105, 0.706178, 0.178437, 0.553657);
    lut->SetTableValue(106, 0.710549, 0.182868, 0.550004);
    lut->SetTableValue(107, 0.714883, 0.187299, 0.546338);
    lut->SetTableValue(108, 0.719181, 0.191729, 0.542663);
    lut->SetTableValue(109, 0.723444, 0.196158, 0.538981);
    lut->SetTableValue(110, 0.72767, 0.200586, 0.535293);
    lut->SetTableValue(111, 0.731862, 0.205013, 0.531601);
    lut->SetTableValue(112, 0.736019, 0.209439, 0.527908);
    lut->SetTableValue(113, 0.740143, 0.213864, 0.524216);
    lut->SetTableValue(114, 0.744232, 0.218288, 0.520524);
    lut->SetTableValue(115, 0.748289, 0.222711, 0.516834);
    lut->SetTableValue(116, 0.752312, 0.227133, 0.513149);
    lut->SetTableValue(117, 0.756304, 0.231555, 0.509468);
    lut->SetTableValue(118, 0.760264, 0.235976, 0.505794);
    lut->SetTableValue(119, 0.764193, 0.240396, 0.502126);
    lut->SetTableValue(120, 0.76809, 0.244817, 0.498465);
    lut->SetTableValue(121, 0.771958, 0.249237, 0.494813);
    lut->SetTableValue(122, 0.775796, 0.253658, 0.491171);
    lut->SetTableValue(123, 0.779604, 0.258078, 0.487539);
    lut->SetTableValue(124, 0.783383, 0.2625, 0.483918);
    lut->SetTableValue(125, 0.787133, 0.266922, 0.480307);
    lut->SetTableValue(126, 0.790855, 0.271345, 0.476706);
    lut->SetTableValue(127, 0.794549, 0.27577, 0.473117);
    lut->SetTableValue(128, 0.798216, 0.280197, 0.469538);
    lut->SetTableValue(129, 0.801855, 0.284626, 0.465971);
    lut->SetTableValue(130, 0.805467, 0.289057, 0.462415);
    lut->SetTableValue(131, 0.809052, 0.293491, 0.45887);
    lut->SetTableValue(132, 0.812612, 0.297928, 0.455338);
    lut->SetTableValue(133, 0.816144, 0.302368, 0.451816);
    lut->SetTableValue(134, 0.819651, 0.306812, 0.448306);
    lut->SetTableValue(135, 0.823132, 0.311261, 0.444806);
    lut->SetTableValue(136, 0.826588, 0.315714, 0.441316);
    lut->SetTableValue(137, 0.830018, 0.320172, 0.437836);
    lut->SetTableValue(138, 0.833422, 0.324635, 0.434366);
    lut->SetTableValue(139, 0.836801, 0.329105, 0.430905);
    lut->SetTableValue(140, 0.840155, 0.33358, 0.427455);
    lut->SetTableValue(141, 0.843484, 0.338062, 0.424013);
    lut->SetTableValue(142, 0.846788, 0.342551, 0.420579);
    lut->SetTableValue(143, 0.850066, 0.347048, 0.417153);
    lut->SetTableValue(144, 0.853319, 0.351553, 0.413734);
    lut->SetTableValue(145, 0.856547, 0.356066, 0.410322);
    lut->SetTableValue(146, 0.85975, 0.360588, 0.406917);
    lut->SetTableValue(147, 0.862927, 0.365119, 0.403519);
    lut->SetTableValue(148, 0.866078, 0.36966, 0.400126);
    lut->SetTableValue(149, 0.869203, 0.374212, 0.396738);
    lut->SetTableValue(150, 0.872303, 0.378774, 0.393355);
    lut->SetTableValue(151, 0.875376, 0.383347, 0.389976);
    lut->SetTableValue(152, 0.878423, 0.387932, 0.3866);
    lut->SetTableValue(153, 0.881443, 0.392529, 0.383229);
    lut->SetTableValue(154, 0.884436, 0.397139, 0.37986);
    lut->SetTableValue(155, 0.887402, 0.401762, 0.376494);
    lut->SetTableValue(156, 0.89034, 0.406398, 0.37313);
    lut->SetTableValue(157, 0.89325, 0.411048, 0.369768);
    lut->SetTableValue(158, 0.896131, 0.415712, 0.366407);
    lut->SetTableValue(159, 0.898984, 0.420392, 0.363047);
    lut->SetTableValue(160, 0.901807, 0.425087, 0.359688);
    lut->SetTableValue(161, 0.904601, 0.429797, 0.356329);
    lut->SetTableValue(162, 0.907365, 0.434524, 0.35297);
    lut->SetTableValue(163, 0.910098, 0.439268, 0.34961);
    lut->SetTableValue(164, 0.9128, 0.444029, 0.346251);
    lut->SetTableValue(165, 0.915471, 0.448807, 0.34289);
    lut->SetTableValue(166, 0.918109, 0.453603, 0.339529);
    lut->SetTableValue(167, 0.920714, 0.458417, 0.336166);
    lut->SetTableValue(168, 0.923287, 0.463251, 0.332801);
    lut->SetTableValue(169, 0.925825, 0.468103, 0.329435);
    lut->SetTableValue(170, 0.928329, 0.472975, 0.326067);
    lut->SetTableValue(171, 0.930798, 0.477867, 0.322697);
    lut->SetTableValue(172, 0.933232, 0.48278, 0.319325);
    lut->SetTableValue(173, 0.93563, 0.487712, 0.315952);
    lut->SetTableValue(174, 0.93799, 0.492667, 0.312575);
    lut->SetTableValue(175, 0.940313, 0.497642, 0.309197);
    lut->SetTableValue(176, 0.942598, 0.502639, 0.305816);
    lut->SetTableValue(177, 0.944844, 0.507658, 0.302433);
    lut->SetTableValue(178, 0.947051, 0.512699, 0.299049);
    lut->SetTableValue(179, 0.949217, 0.517763, 0.295662);
    lut->SetTableValue(180, 0.951344, 0.52285, 0.292275);
    lut->SetTableValue(181, 0.953428, 0.52796, 0.288883);
    lut->SetTableValue(182, 0.95547, 0.533093, 0.28549);
    lut->SetTableValue(183, 0.957469, 0.53825, 0.282096);
    lut->SetTableValue(184, 0.959424, 0.543431, 0.278701);
    lut->SetTableValue(185, 0.961336, 0.548636, 0.275305);
    lut->SetTableValue(186, 0.963203, 0.553865, 0.271909);
    lut->SetTableValue(187, 0.965024, 0.559118, 0.268513);
    lut->SetTableValue(188, 0.966798, 0.564396, 0.265118);
    lut->SetTableValue(189, 0.968526, 0.5697, 0.261721);
    lut->SetTableValue(190, 0.970205, 0.575028, 0.258325);
    lut->SetTableValue(191, 0.971835, 0.580382, 0.254931);
    lut->SetTableValue(192, 0.973416, 0.585761, 0.25154);
    lut->SetTableValue(193, 0.974947, 0.591165, 0.248151);
    lut->SetTableValue(194, 0.976428, 0.596595, 0.244767);
    lut->SetTableValue(195, 0.977856, 0.602051, 0.241387);
    lut->SetTableValue(196, 0.979233, 0.607532, 0.238013);
    lut->SetTableValue(197, 0.980556, 0.613039, 0.234646);
    lut->SetTableValue(198, 0.981826, 0.618572, 0.231287);
    lut->SetTableValue(199, 0.983041, 0.624131, 0.227937);
    lut->SetTableValue(200, 0.984199, 0.629718, 0.224595);
    lut->SetTableValue(201, 0.985301, 0.63533, 0.221265);
    lut->SetTableValue(202, 0.986345, 0.640969, 0.217948);
    lut->SetTableValue(203, 0.987332, 0.646633, 0.214648);
    lut->SetTableValue(204, 0.98826, 0.652325, 0.211364);
    lut->SetTableValue(205, 0.989128, 0.658043, 0.2081);
    lut->SetTableValue(206, 0.989935, 0.663787, 0.204859);
    lut->SetTableValue(207, 0.990681, 0.669558, 0.201642);
    lut->SetTableValue(208, 0.991365, 0.675355, 0.198453);
    lut->SetTableValue(209, 0.991985, 0.681179, 0.195295);
    lut->SetTableValue(210, 0.992541, 0.68703, 0.19217);
    lut->SetTableValue(211, 0.993032, 0.692907, 0.189084);
    lut->SetTableValue(212, 0.993456, 0.69881, 0.186041);
    lut->SetTableValue(213, 0.993814, 0.704741, 0.183043);
    lut->SetTableValue(214, 0.994103, 0.710698, 0.180097);
    lut->SetTableValue(215, 0.994324, 0.716681, 0.177208);
    lut->SetTableValue(216, 0.994474, 0.722691, 0.174381);
    lut->SetTableValue(217, 0.994553, 0.728728, 0.171622);
    lut->SetTableValue(218, 0.994561, 0.734791, 0.168938);
    lut->SetTableValue(219, 0.994495, 0.74088, 0.166335);
    lut->SetTableValue(220, 0.994355, 0.746995, 0.163821);
    lut->SetTableValue(221, 0.994141, 0.753137, 0.161404);
    lut->SetTableValue(222, 0.993851, 0.759304, 0.159092);
    lut->SetTableValue(223, 0.993482, 0.765499, 0.156891);
    lut->SetTableValue(224, 0.993033, 0.77172, 0.154808);
    lut->SetTableValue(225, 0.992505, 0.777967, 0.152855);
    lut->SetTableValue(226, 0.991897, 0.784239, 0.151042);
    lut->SetTableValue(227, 0.991209, 0.790537, 0.149377);
    lut->SetTableValue(228, 0.990439, 0.796859, 0.14787);
    lut->SetTableValue(229, 0.989587, 0.803205, 0.146529);
    lut->SetTableValue(230, 0.988648, 0.809579, 0.145357);
    lut->SetTableValue(231, 0.987621, 0.815978, 0.144363);
    lut->SetTableValue(232, 0.986509, 0.822401, 0.143557);
    lut->SetTableValue(233, 0.985314, 0.828846, 0.142945);
    lut->SetTableValue(234, 0.984031, 0.835315, 0.142528);
    lut->SetTableValue(235, 0.982653, 0.841812, 0.142303);
    lut->SetTableValue(236, 0.98119, 0.848329, 0.142279);
    lut->SetTableValue(237, 0.979644, 0.854866, 0.142453);
    lut->SetTableValue(238, 0.977995, 0.861432, 0.142808);
    lut->SetTableValue(239, 0.976265, 0.868016, 0.143351);
    lut->SetTableValue(240, 0.974443, 0.874622, 0.144061);
    lut->SetTableValue(241, 0.97253, 0.88125, 0.144923);
    lut->SetTableValue(242, 0.970533, 0.887896, 0.145919);
    lut->SetTableValue(243, 0.968443, 0.894564, 0.147014);
    lut->SetTableValue(244, 0.966271, 0.901249, 0.14818);
    lut->SetTableValue(245, 0.964021, 0.90795, 0.14937);
    lut->SetTableValue(246, 0.961681, 0.914672, 0.15052);
    lut->SetTableValue(247, 0.959276, 0.921407, 0.151566);
    lut->SetTableValue(248, 0.956808, 0.928152, 0.152409);
    lut->SetTableValue(249, 0.954287, 0.934908, 0.152921);
    lut->SetTableValue(250, 0.951726, 0.941671, 0.152925);
    lut->SetTableValue(251, 0.949151, 0.948435, 0.152178);
    lut->SetTableValue(252, 0.946602, 0.95519, 0.150328);
    lut->SetTableValue(253, 0.944152, 0.961916, 0.146861);
    lut->SetTableValue(254, 0.941896, 0.96859, 0.140956);
    lut->SetTableValue(255, 0.940015, 0.975158, 0.131326);
}

void ColorMaps::SetCividis(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    lut->SetTableValue(0, 0.0, 0.135112, 0.304751);
    lut->SetTableValue(1, 0.0, 0.138068, 0.311105);
    lut->SetTableValue(2, 0.0, 0.141013, 0.317579);
    lut->SetTableValue(3, 0.0, 0.143951, 0.323982);
    lut->SetTableValue(4, 0.0, 0.146877, 0.330479);
    lut->SetTableValue(5, 0.0, 0.149791, 0.337065);
    lut->SetTableValue(6, 0.0, 0.152673, 0.343704);
    lut->SetTableValue(7, 0.0, 0.155377, 0.3505);
    lut->SetTableValue(8, 0.0, 0.157932, 0.357521);
    lut->SetTableValue(9, 0.0, 0.160495, 0.364534);
    lut->SetTableValue(10, 0.0, 0.163058, 0.371608);
    lut->SetTableValue(11, 0.0, 0.165621, 0.378769);
    lut->SetTableValue(12, 0.0, 0.168204, 0.385902);
    lut->SetTableValue(13, 0.0, 0.1708, 0.3931);
    lut->SetTableValue(14, 0.0, 0.17342, 0.400353);
    lut->SetTableValue(15, 0.0, 0.176082, 0.407577);
    lut->SetTableValue(16, 0.0, 0.178802, 0.414764);
    lut->SetTableValue(17, 0.0, 0.18161, 0.421859);
    lut->SetTableValue(18, 0.0, 0.18455, 0.428802);
    lut->SetTableValue(19, 0.0, 0.186915, 0.435532);
    lut->SetTableValue(20, 0.0, 0.188769, 0.439563);
    lut->SetTableValue(21, 0.0, 0.19095, 0.441085);
    lut->SetTableValue(22, 0.0, 0.193366, 0.441561);
    lut->SetTableValue(23, 0.003602, 0.195911, 0.441564);
    lut->SetTableValue(24, 0.017852, 0.198528, 0.441248);
    lut->SetTableValue(25, 0.03211, 0.201199, 0.440785);
    lut->SetTableValue(26, 0.046205, 0.203903, 0.440196);
    lut->SetTableValue(27, 0.058378, 0.206629, 0.439531);
    lut->SetTableValue(28, 0.068968, 0.209372, 0.438863);
    lut->SetTableValue(29, 0.078624, 0.212122, 0.438105);
    lut->SetTableValue(30, 0.087465, 0.214879, 0.437342);
    lut->SetTableValue(31, 0.095645, 0.217643, 0.436593);
    lut->SetTableValue(32, 0.103401, 0.220406, 0.43579);
    lut->SetTableValue(33, 0.110658, 0.22317, 0.435067);
    lut->SetTableValue(34, 0.117612, 0.225935, 0.434308);
    lut->SetTableValue(35, 0.124291, 0.228697, 0.433547);
    lut->SetTableValue(36, 0.130669, 0.231458, 0.43284);
    lut->SetTableValue(37, 0.13683, 0.234216, 0.432148);
    lut->SetTableValue(38, 0.142852, 0.236972, 0.431404);
    lut->SetTableValue(39, 0.148638, 0.239724, 0.430752);
    lut->SetTableValue(40, 0.154261, 0.242475, 0.43012);
    lut->SetTableValue(41, 0.159733, 0.245221, 0.429528);
    lut->SetTableValue(42, 0.165113, 0.247965, 0.428908);
    lut->SetTableValue(43, 0.170362, 0.250707, 0.428325);
    lut->SetTableValue(44, 0.17549, 0.253444, 0.42779);
    lut->SetTableValue(45, 0.180503, 0.25618, 0.427299);
    lut->SetTableValue(46, 0.185453, 0.258914, 0.426788);
    lut->SetTableValue(47, 0.190303, 0.261644, 0.426329);
    lut->SetTableValue(48, 0.195057, 0.264372, 0.425924);
    lut->SetTableValue(49, 0.199764, 0.267099, 0.425497);
    lut->SetTableValue(50, 0.204385, 0.269823, 0.425126);
    lut->SetTableValue(51, 0.208926, 0.272546, 0.424809);
    lut->SetTableValue(52, 0.213431, 0.275266, 0.42448);
    lut->SetTableValue(53, 0.217863, 0.277985, 0.424206);
    lut->SetTableValue(54, 0.222264, 0.280702, 0.423914);
    lut->SetTableValue(55, 0.226598, 0.283419, 0.423678);
    lut->SetTableValue(56, 0.230871, 0.286134, 0.423498);
    lut->SetTableValue(57, 0.23512, 0.288848, 0.423304);
    lut->SetTableValue(58, 0.239312, 0.291562, 0.423167);
    lut->SetTableValue(59, 0.243485, 0.294274, 0.423014);
    lut->SetTableValue(60, 0.247605, 0.296986, 0.422917);
    lut->SetTableValue(61, 0.251675, 0.299698, 0.422873);
    lut->SetTableValue(62, 0.255731, 0.302409, 0.422814);
    lut->SetTableValue(63, 0.25974, 0.30512, 0.42281);
    lut->SetTableValue(64, 0.263738, 0.307831, 0.422789);
    lut->SetTableValue(65, 0.267693, 0.310542, 0.422821);
    lut->SetTableValue(66, 0.271639, 0.313253, 0.422837);
    lut->SetTableValue(67, 0.275513, 0.315965, 0.422979);
    lut->SetTableValue(68, 0.279411, 0.318677, 0.423031);
    lut->SetTableValue(69, 0.28324, 0.32139, 0.423211);
    lut->SetTableValue(70, 0.287065, 0.324103, 0.423373);
    lut->SetTableValue(71, 0.290884, 0.326816, 0.423517);
    lut->SetTableValue(72, 0.294669, 0.329531, 0.423716);
    lut->SetTableValue(73, 0.298421, 0.332247, 0.423973);
    lut->SetTableValue(74, 0.302169, 0.334963, 0.424213);
    lut->SetTableValue(75, 0.305886, 0.337681, 0.424512);
    lut->SetTableValue(76, 0.309601, 0.340399, 0.42479);
    lut->SetTableValue(77, 0.313287, 0.34312, 0.42512);
    lut->SetTableValue(78, 0.316941, 0.345842, 0.425512);
    lut->SetTableValue(79, 0.320595, 0.348565, 0.425889);
    lut->SetTableValue(80, 0.32425, 0.351289, 0.42625);
    lut->SetTableValue(81, 0.327875, 0.354016, 0.42667);
    lut->SetTableValue(82, 0.331474, 0.356744, 0.427144);
    lut->SetTableValue(83, 0.335073, 0.359474, 0.427605);
    lut->SetTableValue(84, 0.338673, 0.362206, 0.428053);
    lut->SetTableValue(85, 0.342246, 0.364939, 0.428559);
    lut->SetTableValue(86, 0.345793, 0.367676, 0.429127);
    lut->SetTableValue(87, 0.349341, 0.370414, 0.429685);
    lut->SetTableValue(88, 0.352892, 0.373153, 0.430226);
    lut->SetTableValue(89, 0.356418, 0.375896, 0.430823);
    lut->SetTableValue(90, 0.359916, 0.378641, 0.431501);
    lut->SetTableValue(91, 0.363446, 0.381388, 0.432075);
    lut->SetTableValue(92, 0.366923, 0.384139, 0.432796);
    lut->SetTableValue(93, 0.37043, 0.38689, 0.433428);
    lut->SetTableValue(94, 0.373884, 0.389646, 0.434209);
    lut->SetTableValue(95, 0.377371, 0.392404, 0.43489);
    lut->SetTableValue(96, 0.38083, 0.395164, 0.435653);
    lut->SetTableValue(97, 0.384268, 0.397928, 0.436475);
    lut->SetTableValue(98, 0.387705, 0.400694, 0.437305);
    lut->SetTableValue(99, 0.391151, 0.403464, 0.438096);
    lut->SetTableValue(100, 0.394568, 0.406236, 0.438986);
    lut->SetTableValue(101, 0.397991, 0.409011, 0.439848);
    lut->SetTableValue(102, 0.401418, 0.41179, 0.440708);
    lut->SetTableValue(103, 0.40482, 0.414572, 0.441642);
    lut->SetTableValue(104, 0.408226, 0.417357, 0.44257);
    lut->SetTableValue(105, 0.411607, 0.420145, 0.443577);
    lut->SetTableValue(106, 0.414992, 0.422937, 0.444578);
    lut->SetTableValue(107, 0.418383, 0.425733, 0.44556);
    lut->SetTableValue(108, 0.421748, 0.428531, 0.44664);
    lut->SetTableValue(109, 0.42512, 0.431334, 0.447692);
    lut->SetTableValue(110, 0.428462, 0.43414, 0.448864);
    lut->SetTableValue(111, 0.431817, 0.43695, 0.449982);
    lut->SetTableValue(112, 0.435168, 0.439763, 0.451134);
    lut->SetTableValue(113, 0.438504, 0.44258, 0.452341);
    lut->SetTableValue(114, 0.44181, 0.445402, 0.453659);
    lut->SetTableValue(115, 0.445148, 0.448226, 0.454885);
    lut->SetTableValue(116, 0.448447, 0.451053, 0.456264);
    lut->SetTableValue(117, 0.451759, 0.453887, 0.457582);
    lut->SetTableValue(118, 0.455072, 0.456718, 0.458976);
    lut->SetTableValue(119, 0.458366, 0.459552, 0.460457);
    lut->SetTableValue(120, 0.461616, 0.462405, 0.461969);
    lut->SetTableValue(121, 0.464947, 0.465241, 0.463395);
    lut->SetTableValue(122, 0.468254, 0.468083, 0.464908);
    lut->SetTableValue(123, 0.471501, 0.47096, 0.466357);
    lut->SetTableValue(124, 0.474812, 0.473832, 0.467681);
    lut->SetTableValue(125, 0.478186, 0.476699, 0.468845);
    lut->SetTableValue(126, 0.481622, 0.479573, 0.469767);
    lut->SetTableValue(127, 0.485141, 0.482451, 0.470384);
    lut->SetTableValue(128, 0.488697, 0.485318, 0.471008);
    lut->SetTableValue(129, 0.492278, 0.488198, 0.471453);
    lut->SetTableValue(130, 0.495913, 0.491076, 0.471751);
    lut->SetTableValue(131, 0.499552, 0.49396, 0.472032);
    lut->SetTableValue(132, 0.503185, 0.496851, 0.472305);
    lut->SetTableValue(133, 0.506866, 0.499743, 0.472432);
    lut->SetTableValue(134, 0.51054, 0.502643, 0.47255);
    lut->SetTableValue(135, 0.514226, 0.505546, 0.47264);
    lut->SetTableValue(136, 0.51792, 0.508454, 0.472707);
    lut->SetTableValue(137, 0.521643, 0.511367, 0.472639);
    lut->SetTableValue(138, 0.525348, 0.514285, 0.47266);
    lut->SetTableValue(139, 0.529086, 0.517207, 0.472543);
    lut->SetTableValue(140, 0.532829, 0.520135, 0.472401);
    lut->SetTableValue(141, 0.536553, 0.523067, 0.472352);
    lut->SetTableValue(142, 0.540307, 0.526005, 0.472163);
    lut->SetTableValue(143, 0.544069, 0.528948, 0.471947);
    lut->SetTableValue(144, 0.54784, 0.531895, 0.471704);
    lut->SetTableValue(145, 0.551612, 0.534849, 0.471439);
    lut->SetTableValue(146, 0.555393, 0.537807, 0.471147);
    lut->SetTableValue(147, 0.559181, 0.540771, 0.470829);
    lut->SetTableValue(148, 0.562972, 0.543741, 0.470488);
    lut->SetTableValue(149, 0.566802, 0.546715, 0.469988);
    lut->SetTableValue(150, 0.570607, 0.549695, 0.469593);
    lut->SetTableValue(151, 0.574417, 0.552682, 0.469172);
    lut->SetTableValue(152, 0.578236, 0.555673, 0.468724);
    lut->SetTableValue(153, 0.582087, 0.55867, 0.468118);
    lut->SetTableValue(154, 0.585916, 0.561674, 0.467618);
    lut->SetTableValue(155, 0.589753, 0.564682, 0.46709);
    lut->SetTableValue(156, 0.593622, 0.567697, 0.466401);
    lut->SetTableValue(157, 0.597469, 0.570718, 0.465821);
    lut->SetTableValue(158, 0.601354, 0.573743, 0.465074);
    lut->SetTableValue(159, 0.605211, 0.576777, 0.464441);
    lut->SetTableValue(160, 0.609105, 0.579816, 0.463638);
    lut->SetTableValue(161, 0.612977, 0.582861, 0.46295);
    lut->SetTableValue(162, 0.616852, 0.585913, 0.462237);
    lut->SetTableValue(163, 0.620765, 0.58897, 0.461351);
    lut->SetTableValue(164, 0.624654, 0.592034, 0.460583);
    lut->SetTableValue(165, 0.628576, 0.595104, 0.459641);
    lut->SetTableValue(166, 0.632506, 0.59818, 0.458668);
    lut->SetTableValue(167, 0.636412, 0.601264, 0.457818);
    lut->SetTableValue(168, 0.640352, 0.604354, 0.456791);
    lut->SetTableValue(169, 0.64427, 0.60745, 0.455886);
    lut->SetTableValue(170, 0.648222, 0.610553, 0.454801);
    lut->SetTableValue(171, 0.652178, 0.613664, 0.453689);
    lut->SetTableValue(172, 0.656114, 0.61678, 0.452702);
    lut->SetTableValue(173, 0.660082, 0.619904, 0.451534);
    lut->SetTableValue(174, 0.664055, 0.623034, 0.450338);
    lut->SetTableValue(175, 0.668008, 0.626171, 0.44927);
    lut->SetTableValue(176, 0.671991, 0.629316, 0.448018);
    lut->SetTableValue(177, 0.675981, 0.632468, 0.446736);
    lut->SetTableValue(178, 0.679979, 0.635626, 0.445424);
    lut->SetTableValue(179, 0.68395, 0.638793, 0.444251);
    lut->SetTableValue(180, 0.687957, 0.641966, 0.442886);
    lut->SetTableValue(181, 0.691971, 0.645145, 0.441491);
    lut->SetTableValue(182, 0.695985, 0.648334, 0.440072);
    lut->SetTableValue(183, 0.700008, 0.651529, 0.438624);
    lut->SetTableValue(184, 0.704037, 0.654731, 0.437147);
    lut->SetTableValue(185, 0.708067, 0.657942, 0.435647);
    lut->SetTableValue(186, 0.712105, 0.66116, 0.434117);
    lut->SetTableValue(187, 0.716177, 0.664384, 0.432386);
    lut->SetTableValue(188, 0.720222, 0.667618, 0.430805);
    lut->SetTableValue(189, 0.724274, 0.670859, 0.429194);
    lut->SetTableValue(190, 0.728334, 0.674107, 0.427554);
    lut->SetTableValue(191, 0.732422, 0.677364, 0.425717);
    lut->SetTableValue(192, 0.736488, 0.680629, 0.424028);
    lut->SetTableValue(193, 0.740589, 0.6839, 0.422131);
    lut->SetTableValue(194, 0.744664, 0.687181, 0.420393);
    lut->SetTableValue(195, 0.748772, 0.69047, 0.418448);
    lut->SetTableValue(196, 0.752886, 0.693766, 0.416472);
    lut->SetTableValue(197, 0.756975, 0.697071, 0.414659);
    lut->SetTableValue(198, 0.761096, 0.700384, 0.412638);
    lut->SetTableValue(199, 0.765223, 0.703705, 0.410587);
    lut->SetTableValue(200, 0.769353, 0.707035, 0.408516);
    lut->SetTableValue(201, 0.773486, 0.710373, 0.406422);
    lut->SetTableValue(202, 0.777651, 0.713719, 0.404112);
    lut->SetTableValue(203, 0.781795, 0.717074, 0.401966);
    lut->SetTableValue(204, 0.785965, 0.720438, 0.399613);
    lut->SetTableValue(205, 0.790116, 0.72381, 0.397423);
    lut->SetTableValue(206, 0.794298, 0.72719, 0.395016);
    lut->SetTableValue(207, 0.79848, 0.73058, 0.392597);
    lut->SetTableValue(208, 0.802667, 0.733978, 0.390153);
    lut->SetTableValue(209, 0.806859, 0.737385, 0.387684);
    lut->SetTableValue(210, 0.811054, 0.740801, 0.385198);
    lut->SetTableValue(211, 0.815274, 0.744226, 0.382504);
    lut->SetTableValue(212, 0.819499, 0.747659, 0.379785);
    lut->SetTableValue(213, 0.823729, 0.751101, 0.377043);
    lut->SetTableValue(214, 0.827959, 0.754553, 0.374292);
    lut->SetTableValue(215, 0.832192, 0.758014, 0.371529);
    lut->SetTableValue(216, 0.836429, 0.761483, 0.368747);
    lut->SetTableValue(217, 0.840693, 0.764962, 0.365746);
    lut->SetTableValue(218, 0.844957, 0.76845, 0.362741);
    lut->SetTableValue(219, 0.849223, 0.771947, 0.359729);
    lut->SetTableValue(220, 0.853515, 0.775454, 0.3565);
    lut->SetTableValue(221, 0.857809, 0.778969, 0.353259);
    lut->SetTableValue(222, 0.862105, 0.782494, 0.350011);
    lut->SetTableValue(223, 0.866421, 0.786028, 0.346571);
    lut->SetTableValue(224, 0.870717, 0.789572, 0.343333);
    lut->SetTableValue(225, 0.875057, 0.793125, 0.339685);
    lut->SetTableValue(226, 0.879378, 0.796687, 0.336241);
    lut->SetTableValue(227, 0.88372, 0.800258, 0.332599);
    lut->SetTableValue(228, 0.888081, 0.803839, 0.32877);
    lut->SetTableValue(229, 0.89244, 0.80743, 0.324968);
    lut->SetTableValue(230, 0.896818, 0.81103, 0.320982);
    lut->SetTableValue(231, 0.901195, 0.814639, 0.317021);
    lut->SetTableValue(232, 0.905589, 0.818257, 0.312889);
    lut->SetTableValue(233, 0.91, 0.821885, 0.308594);
    lut->SetTableValue(234, 0.914407, 0.825522, 0.304348);
    lut->SetTableValue(235, 0.918828, 0.829168, 0.29996);
    lut->SetTableValue(236, 0.923279, 0.832822, 0.295244);
    lut->SetTableValue(237, 0.927724, 0.836486, 0.290611);
    lut->SetTableValue(238, 0.93218, 0.840159, 0.28588);
    lut->SetTableValue(239, 0.93666, 0.843841, 0.280876);
    lut->SetTableValue(240, 0.941147, 0.84753, 0.275815);
    lut->SetTableValue(241, 0.945654, 0.851228, 0.270532);
    lut->SetTableValue(242, 0.950178, 0.854933, 0.265085);
    lut->SetTableValue(243, 0.954725, 0.858646, 0.259365);
    lut->SetTableValue(244, 0.959284, 0.862365, 0.253563);
    lut->SetTableValue(245, 0.963872, 0.866089, 0.247445);
    lut->SetTableValue(246, 0.968469, 0.869819, 0.24131);
    lut->SetTableValue(247, 0.973114, 0.87355, 0.234677);
    lut->SetTableValue(248, 0.97778, 0.877281, 0.227954);
    lut->SetTableValue(249, 0.982497, 0.881008, 0.220878);
    lut->SetTableValue(250, 0.987293, 0.884718, 0.213336);
    lut->SetTableValue(251, 0.992218, 0.888385, 0.205468);
    lut->SetTableValue(252, 0.994847, 0.892954, 0.203445);
    lut->SetTableValue(253, 0.995249, 0.898384, 0.207561);
    lut->SetTableValue(254, 0.995503, 0.903866, 0.21237);
    lut->SetTableValue(255, 0.995737, 0.909344, 0.217772);
}

void ColorMaps::SetGray(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, i / 255., i / 255., i / 255.);
    }
}

void ColorMaps::SetDefault(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    double hsv[3] = { 1., 1., 1. };
    double rgba[4] = { 1., 1., 1., 1. };
    for (vtkIdType i = 0; i < 256; ++i) {
        hsv[0] = i / 360.;
        vtkMath::HSVToRGB(hsv, rgba);
        lut->SetTableValue(i, rgba);
    }
}

void ColorMaps::SetDefaultStep(vtkLookupTable *lut)
{
    ColorMaps::SetDefault(lut);
    for (vtkIdType i = 0; i < 256; i += 16) {
        lut->SetTableValue(i, 128. / 255., 128. / 255., 128. / 255.);
    }
}

void ColorMaps::SetMinMax(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    double hsv[3] = { 1., 1., 1. };
    double rgba[4] = { 1., 1., 1., 1. };
    for (vtkIdType i = 0; i < 128; ++i) {
        hsv[0] = i / 360.;
        rgba[3] = (128. - i) / 128.;
        vtkMath::HSVToRGB(hsv, rgba);
        lut->SetTableValue(i, rgba);
    }

    for (vtkIdType i = 128; i < 256; ++i) {
        hsv[0] = i / 360.;
        rgba[3] = (i - 128.) / 128.;
        vtkMath::HSVToRGB(hsv, rgba);
        lut->SetTableValue(i, rgba);
    }
}

void ColorMaps::SetGlow(vtkLookupTable *lut)
{
    unsigned char v[256 * 3] = {
        0,   0,   0,   0,   0,   0,   5,   0,   0,   5,   0,   0,   9,   0,   0,   9,   0,   0,
        13,  0,   0,   13,  0,   5,   17,  0,   5,   17,  0,   5,   21,  0,   5,   21,  0,   5,
        25,  0,   5,   25,  0,   5,   29,  0,   5,   29,  0,   9,   33,  0,   9,   33,  0,   9,
        37,  0,   9,   37,  0,   9,   41,  0,   9,   41,  0,   9,   45,  0,   9,   45,  0,   9,
        49,  0,   13,  49,  0,   13,  53,  0,   13,  53,  0,   13,  57,  0,   13,  57,  0,   13,
        61,  0,   13,  61,  5,   13,  65,  5,   13,  65,  5,   13,  69,  5,   13,  69,  5,   13,
        73,  5,   17,  73,  5,   17,  77,  5,   17,  81,  5,   17,  81,  5,   17,  85,  5,   17,
        85,  5,   17,  89,  5,   17,  89,  9,   17,  93,  9,   17,  93,  9,   17,  97,  9,   17,
        97,  9,   17,  101, 9,   17,  101, 9,   17,  105, 9,   17,  105, 9,   17,  109, 13,  17,
        109, 13,  17,  113, 13,  17,  113, 13,  17,  117, 13,  17,  117, 13,  17,  121, 13,  17,
        125, 13,  17,  125, 13,  17,  128, 17,  17,  128, 17,  17,  132, 17,  21,  132, 17,  21,
        136, 17,  21,  136, 17,  21,  140, 17,  21,  140, 21,  21,  144, 21,  21,  144, 21,  21,
        148, 21,  21,  148, 21,  21,  152, 25,  21,  152, 25,  21,  156, 25,  21,  156, 25,  21,
        156, 25,  21,  160, 29,  21,  160, 29,  21,  164, 29,  21,  164, 29,  21,  168, 33,  21,
        168, 33,  21,  168, 33,  21,  172, 33,  21,  172, 37,  21,  176, 37,  21,  176, 37,  21,
        176, 37,  21,  180, 41,  21,  180, 41,  21,  180, 41,  21,  184, 45,  21,  184, 45,  21,
        188, 45,  21,  188, 49,  21,  188, 49,  21,  192, 49,  21,  192, 53,  21,  192, 53,  21,
        192, 53,  21,  196, 57,  21,  196, 57,  21,  196, 61,  21,  200, 61,  21,  200, 61,  21,
        200, 65,  21,  200, 65,  21,  204, 69,  21,  204, 69,  21,  204, 69,  21,  208, 73,  21,
        208, 73,  21,  208, 77,  21,  208, 77,  21,  208, 81,  21,  212, 81,  21,  212, 85,  21,
        212, 85,  21,  212, 89,  21,  212, 89,  21,  216, 93,  21,  216, 93,  21,  216, 97,  21,
        216, 97,  21,  216, 101, 21,  220, 101, 21,  220, 105, 21,  220, 105, 21,  220, 109, 21,
        220, 109, 21,  220, 113, 21,  220, 113, 21,  224, 117, 21,  224, 117, 25,  224, 121, 25,
        224, 121, 25,  224, 125, 25,  224, 125, 25,  224, 128, 25,  228, 128, 25,  228, 132, 25,
        228, 132, 29,  228, 136, 29,  228, 136, 29,  228, 140, 29,  228, 140, 29,  228, 144, 33,
        232, 144, 33,  232, 144, 33,  232, 148, 33,  232, 148, 33,  232, 152, 37,  232, 152, 37,
        232, 156, 37,  232, 156, 37,  236, 160, 41,  236, 160, 41,  236, 160, 41,  236, 164, 41,
        236, 164, 45,  236, 168, 45,  236, 168, 45,  236, 172, 49,  236, 172, 49,  236, 172, 49,
        240, 176, 53,  240, 176, 53,  240, 180, 53,  240, 180, 57,  240, 180, 57,  240, 184, 61,
        240, 184, 61,  240, 188, 61,  240, 188, 65,  240, 188, 65,  240, 192, 69,  244, 192, 69,
        244, 192, 73,  244, 192, 73,  244, 192, 77,  244, 200, 77,  244, 200, 81,  244, 200, 81,
        244, 204, 85,  244, 204, 85,  244, 204, 89,  244, 208, 89,  244, 208, 93,  244, 208, 93,
        248, 212, 97,  248, 212, 97,  248, 212, 101, 248, 216, 105, 248, 216, 105, 248, 216, 109,
        248, 220, 109, 248, 220, 113, 248, 220, 113, 248, 224, 117, 248, 224, 121, 248, 224, 121,
        248, 224, 125, 248, 228, 125, 248, 228, 129, 248, 228, 129, 248, 228, 133, 248, 232, 137,
        248, 232, 137, 252, 232, 141, 252, 232, 145, 252, 236, 145, 252, 236, 148, 252, 236, 148,
        252, 236, 152, 252, 240, 156, 252, 240, 156, 252, 240, 160, 252, 240, 160, 252, 240, 164,
        252, 240, 168, 252, 244, 168, 252, 244, 172, 252, 244, 172, 252, 244, 176, 252, 244, 180,
        252, 244, 180, 252, 248, 184, 252, 248, 188, 252, 248, 188, 252, 248, 192, 252, 248, 192,
        252, 248, 196, 252, 248, 200, 252, 248, 200, 252, 252, 204, 252, 252, 208, 252, 252, 208,
        252, 252, 212, 252, 252, 212, 252, 252, 216, 252, 252, 220, 252, 252, 220, 252, 252, 224,
        252, 252, 228, 252, 252, 228, 252, 252, 232, 252, 252, 236, 252, 252, 236, 252, 252, 240,
        252, 252, 244, 252, 252, 244, 252, 252, 248, 252, 252, 255
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 3;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255.);
    }
}

void ColorMaps::SetTemperature(vtkLookupTable *lut)
{
    unsigned char v[256 * 3] = {
        255, 255, 255, 251, 251, 253, 249, 247, 255, 246, 244, 253, 242, 242, 255, 239, 239, 253,
        237, 235, 255, 233, 232, 255, 230, 230, 253, 228, 226, 255, 225, 223, 253, 221, 221, 255,
        219, 218, 253, 216, 214, 255, 212, 212, 255, 209, 209, 255, 207, 205, 255, 205, 202, 253,
        202, 200, 255, 198, 196, 253, 195, 193, 255, 193, 191, 253, 189, 188, 255, 186, 184, 255,
        184, 181, 255, 181, 179, 255, 177, 175, 253, 175, 172, 255, 172, 170, 253, 168, 167, 255,
        165, 163, 255, 163, 161, 255, 160, 158, 255, 156, 154, 253, 154, 151, 255, 151, 149, 253,
        149, 144, 255, 145, 142, 253, 142, 138, 255, 140, 135, 255, 137, 133, 255, 133, 130, 255,
        130, 126, 253, 128, 124, 255, 124, 121, 253, 121, 117, 255, 119, 114, 255, 116, 112, 255,
        112, 109, 255, 110, 105, 253, 107, 103, 255, 105, 100, 253, 102, 96,  255, 98,  93,  253,
        96,  91,  255, 93,  87,  255, 89,  84,  253, 86,  82,  255, 84,  79,  253, 80,  75,  255,
        77,  73,  253, 75,  70,  255, 72,  66,  255, 68,  63,  255, 66,  61,  255, 66,  59,  253,
        68,  59,  251, 70,  59,  249, 70,  58,  249, 72,  58,  247, 73,  56,  246, 73,  56,  244,
        75,  56,  244, 77,  54,  242, 77,  54,  240, 79,  54,  239, 79,  52,  237, 80,  52,  237,
        82,  52,  235, 84,  52,  233, 84,  51,  232, 86,  49,  232, 86,  49,  230, 87,  49,  228,
        89,  49,  228, 91,  47,  226, 91,  47,  225, 93,  47,  223, 93,  45,  221, 96,  45,  221,
        96,  45,  219, 98,  43,  218, 98,  43,  216, 100, 42,  214, 100, 42,  214, 103, 42,  212,
        103, 40,  211, 105, 40,  209, 105, 40,  209, 107, 38,  207, 107, 38,  205, 110, 38,  205,
        110, 38,  202, 112, 36,  202, 112, 35,  200, 114, 35,  198, 114, 35,  198, 117, 33,  196,
        117, 33,  195, 119, 33,  193, 119, 33,  193, 121, 31,  191, 121, 31,  189, 124, 31,  188,
        124, 29,  186, 126, 29,  186, 126, 28,  184, 128, 28,  182, 128, 28,  181, 130, 26,  181,
        131, 26,  179, 133, 26,  177, 133, 24,  175, 135, 24,  175, 135, 24,  174, 137, 24,  172,
        138, 22,  170, 140, 22,  170, 140, 22,  168, 142, 21,  167, 142, 19,  165, 144, 19,  163,
        145, 19,  163, 147, 19,  161, 147, 17,  160, 149, 17,  158, 149, 17,  158, 151, 15,  156,
        153, 15,  154, 154, 15,  154, 154, 14,  151, 156, 14,  151, 156, 12,  149, 158, 12,  147,
        160, 12,  147, 161, 10,  145, 161, 10,  144, 163, 10,  142, 165, 10,  142, 167, 8,   137,
        168, 8,   135, 172, 8,   131, 174, 8,   128, 175, 8,   126, 179, 8,   121, 181, 8,   119,
        182, 7,   116, 186, 7,   112, 188, 7,   110, 191, 7,   107, 193, 5,   103, 195, 5,   100,
        198, 5,   96,  200, 5,   93,  202, 5,   91,  205, 5,   86,  207, 5,   84,  209, 5,   80,
        212, 3,   77,  214, 3,   75,  216, 3,   72,  219, 3,   68,  221, 3,   65,  223, 3,   61,
        226, 3,   59,  228, 1,   56,  230, 1,   52,  232, 1,   49,  235, 1,   45,  237, 1,   42,
        239, 1,   40,  242, 1,   36,  244, 1,   33,  246, 0,   29,  249, 0,   26,  249, 0,   26,
        249, 0,   26,  249, 0,   26,  251, 0,   26,  251, 0,   24,  251, 0,   24,  251, 0,   24,
        251, 0,   24,  251, 0,   24,  251, 0,   24,  251, 0,   22,  253, 0,   22,  253, 0,   22,
        253, 0,   22,  253, 0,   22,  253, 0,   22,  253, 0,   21,  253, 0,   21,  253, 0,   21,
        255, 0,   21,  255, 3,   21,  255, 8,   21,  255, 12,  21,  255, 17,  21,  255, 22,  21,
        255, 26,  22,  255, 31,  22,  255, 35,  22,  255, 40,  22,  255, 45,  22,  255, 49,  22,
        255, 54,  22,  255, 59,  22,  255, 63,  22,  255, 68,  22,  255, 73,  22,  255, 77,  24,
        255, 82,  24,  255, 86,  24,  255, 91,  24,  255, 96,  24,  255, 100, 24,  255, 105, 24,
        255, 110, 24,  255, 114, 24,  255, 119, 24,  255, 123, 24,  255, 128, 26,  255, 133, 26,
        255, 137, 24,  255, 142, 24,  255, 147, 24,  255, 154, 22,  255, 158, 22,  255, 163, 22,
        255, 168, 21,  255, 175, 21,  255, 179, 19,  255, 184, 19,  255, 191, 19,  255, 195, 17,
        255, 200, 17,  255, 205, 17,  255, 212, 17,  255, 216, 15,  255, 221, 15,  255, 226, 15,
        255, 232, 14,  255, 237, 14,  255, 242, 12,  255, 249, 12
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 3;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255.);
    }
}

void ColorMaps::SetSar(vtkLookupTable *lut)
{
    unsigned char v[256 * 3] = {
        255, 255, 255, 251, 251, 253, 246, 246, 253, 244, 246, 253, 242, 242, 253, 239, 239, 251,
        235, 237, 251, 232, 232, 251, 230, 232, 251, 226, 228, 249, 223, 226, 249, 221, 223, 249,
        218, 219, 249, 214, 219, 246, 212, 214, 246, 209, 214, 246, 207, 212, 246, 204, 209, 246,
        200, 207, 246, 198, 205, 246, 195, 204, 244, 191, 202, 244, 188, 198, 244, 186, 198, 244,
        182, 195, 242, 179, 195, 242, 177, 193, 242, 174, 189, 242, 172, 188, 239, 168, 186, 239,
        167, 186, 239, 163, 186, 239, 161, 184, 239, 158, 181, 239, 156, 179, 239, 153, 181, 237,
        149, 177, 237, 147, 177, 237, 144, 175, 237, 142, 174, 235, 137, 175, 235, 137, 174, 235,
        133, 174, 235, 130, 172, 232, 128, 170, 232, 126, 170, 232, 123, 168, 232, 121, 170, 232,
        117, 168, 232, 116, 167, 232, 112, 168, 230, 110, 167, 230, 107, 167, 230, 105, 167, 230,
        103, 167, 228, 100, 165, 228, 98,  165, 228, 93,  163, 228, 89,  163, 226, 87,  163, 226,
        86,  165, 226, 84,  165, 226, 80,  163, 225, 79,  165, 225, 77,  165, 225, 75,  163, 223,
        73,  165, 223, 68,  163, 223, 66,  163, 223, 65,  167, 221, 61,  167, 221, 59,  165, 221,
        56,  168, 221, 54,  168, 219, 51,  168, 219, 51,  168, 219, 47,  170, 219, 45,  168, 218,
        42,  168, 218, 40,  172, 216, 36,  172, 216, 35,  172, 216, 33,  172, 216, 33,  174, 216,
        28,  175, 214, 28,  174, 214, 24,  175, 214, 24,  175, 214, 21,  179, 212, 17,  179, 212,
        17,  179, 212, 14,  181, 212, 10,  184, 211, 10,  182, 211, 7,   186, 209, 5,   186, 209,
        1,   186, 209, 1,   188, 209, 0,   191, 209, 0,   193, 209, 0,   195, 209, 0,   200, 209,
        0,   200, 209, 0,   204, 209, 0,   207, 209, 0,   211, 209, 0,   211, 207, 0,   211, 205,
        0,   212, 202, 0,   212, 198, 0,   212, 198, 0,   212, 193, 0,   212, 193, 0,   212, 189,
        0,   212, 186, 0,   214, 184, 0,   214, 184, 0,   214, 181, 0,   214, 179, 0,   214, 177,
        0,   214, 174, 0,   214, 172, 0,   216, 167, 0,   216, 165, 0,   216, 163, 0,   216, 161,
        0,   216, 158, 0,   216, 156, 0,   216, 154, 0,   218, 151, 0,   218, 147, 0,   218, 145,
        0,   219, 142, 0,   219, 140, 0,   219, 137, 0,   219, 137, 0,   219, 133, 0,   219, 130,
        0,   219, 128, 0,   221, 124, 0,   221, 123, 0,   221, 121, 0,   221, 117, 0,   221, 114,
        0,   221, 110, 0,   221, 110, 0,   223, 107, 0,   223, 103, 0,   223, 102, 0,   223, 100,
        0,   223, 98,  0,   223, 93,  0,   223, 91,  0,   225, 87,  0,   225, 84,  0,   225, 84,
        0,   226, 80,  0,   226, 77,  0,   226, 73,  0,   226, 72,  0,   226, 70,  0,   226, 68,
        0,   226, 63,  0,   228, 59,  0,   228, 58,  0,   228, 56,  0,   228, 54,  0,   228, 51,
        0,   228, 47,  0,   228, 43,  0,   230, 40,  0,   230, 38,  0,   230, 35,  0,   230, 35,
        0,   230, 29,  0,   230, 28,  0,   230, 26,  0,   232, 22,  0,   232, 17,  0,   232, 15,
        0,   232, 14,  0,   232, 12,  0,   232, 7,   0,   232, 3,   0,   232, 3,   0,   232, 0,
        0,   232, 0,   5,   235, 0,   8,   235, 0,   8,   235, 0,   14,  235, 0,   14,  235, 0,
        19,  235, 0,   24,  235, 0,   31,  237, 0,   38,  237, 0,   45,  237, 0,   51,  237, 0,
        58,  237, 0,   66,  237, 0,   72,  237, 0,   79,  239, 0,   86,  239, 0,   93,  239, 0,
        98,  239, 0,   107, 239, 0,   114, 239, 0,   121, 239, 0,   128, 239, 0,   133, 239, 0,
        140, 239, 0,   149, 242, 0,   154, 242, 0,   161, 242, 0,   170, 242, 0,   177, 242, 0,
        184, 242, 0,   191, 242, 0,   198, 244, 0,   205, 244, 0,   212, 244, 0,   219, 244, 0,
        228, 244, 0,   233, 244, 0,   239, 244, 0,   246, 242, 0,   246, 235, 0,   246, 226, 0,
        246, 221, 0,   246, 214, 0,   246, 205, 0,   246, 198, 0,   246, 193, 0,   246, 186, 0,
        246, 179, 0,   249, 174, 0,   249, 168, 0,   249, 160, 0,   249, 153, 0,   249, 144, 0,
        249, 137, 0,   249, 131, 0,   251, 124, 0,   251, 119, 0,   251, 112, 0,   251, 103, 0,
        251, 96,  0,   251, 89,  0,   251, 82,  0,   253, 77,  0,   253, 70,  0,   253, 61,  0,
        253, 56,  0,   253, 47,  0,   253, 40,  0,   255, 33,  0
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 3;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255.);
    }
}

void ColorMaps::SetPhysicsContour(vtkLookupTable *lut)
{
    unsigned char v[256 * 3] = {
        101, 4,   255, 97,  4,   253, 92,  4,   250, 89,  4,   248, 83,  4,   245, 80,  4,   243,
        76,  4,   241, 73,  4,   238, 68,  4,   236, 65,  2,   233, 62,  2,   231, 57,  2,   229,
        54,  2,   226, 50,  2,   224, 48,  2,   221, 43,  2,   219, 41,  2,   217, 37,  2,   214,
        35,  2,   212, 31,  1,   209, 28,  1,   207, 26,  1,   204, 22,  1,   202, 19,  1,   200,
        16,  1,   197, 13,  1,   195, 12,  0,   192, 9,   0,   190, 6,   0,   188, 4,   0,   185,
        1,   0,   183, 0,   1,   180, 0,   3,   178, 0,   6,   176, 0,   8,   173, 0,   10,  171,
        0,   12,  168, 0,   13,  166, 0,   15,  164, 0,   18,  161, 1,   19,  159, 2,   22,  156,
        3,   24,  154, 3,   25,  152, 4,   28,  149, 4,   29,  147, 5,   31,  144, 6,   33,  142,
        7,   33,  140, 6,   35,  137, 7,   37,  135, 8,   37,  132, 8,   39,  130, 9,   40,  128,
        19,  84,  255, 20,  85,  253, 21,  88,  250, 22,  91,  248, 22,  94,  245, 23,  95,  243,
        24,  97,  241, 25,  100, 238, 25,  102, 236, 26,  103, 233, 27,  105, 231, 28,  107, 229,
        29,  109, 226, 29,  111, 224, 30,  111, 221, 30,  113, 219, 29,  116, 217, 29,  118, 214,
        29,  119, 212, 28,  121, 209, 28,  123, 207, 28,  124, 204, 27,  125, 202, 27,  126, 200,
        27,  128, 197, 26,  129, 195, 26,  131, 192, 26,  132, 190, 25,  133, 188, 25,  134, 185,
        25,  135, 183, 24,  136, 180, 24,  137, 178, 23,  137, 176, 23,  138, 173, 22,  138, 171,
        22,  139, 168, 21,  140, 166, 21,  139, 164, 21,  140, 161, 20,  140, 159, 20,  140, 156,
        20,  141, 154, 19,  140, 152, 19,  140, 149, 19,  140, 147, 18,  140, 144, 18,  140, 142,
        18,  140, 140, 17,  137, 135, 17,  135, 131, 17,  132, 127, 16,  130, 123, 16,  128, 118,
        32,  255, 232, 32,  253, 226, 31,  250, 221, 31,  248, 215, 31,  245, 209, 29,  243, 203,
        29,  241, 198, 29,  238, 193, 28,  236, 187, 28,  233, 181, 28,  231, 176, 27,  229, 171,
        27,  226, 167, 27,  224, 158, 26,  221, 151, 26,  219, 144, 26,  217, 138, 25,  214, 131,
        25,  212, 124, 25,  209, 117, 24,  207, 111, 24,  204, 104, 23,  202, 98,  22,  200, 92,
        22,  197, 86,  22,  195, 79,  21,  192, 74,  21,  190, 68,  21,  188, 63,  21,  185, 57,
        20,  183, 52,  20,  180, 47,  20,  178, 42,  19,  176, 37,  19,  173, 32,  18,  171, 27,
        18,  168, 23,  17,  166, 18,  21,  164, 17,  25,  161, 17,  28,  159, 17,  31,  156, 16,
        35,  154, 16,  38,  152, 16,  41,  149, 15,  44,  147, 15,  47,  144, 15,  49,  142, 14,
        52,  140, 14,  54,  137, 13,  57,  135, 13,  59,  132, 13,  61,  130, 13,  63,  128, 12,
        133, 255, 25,  138, 253, 25,  142, 250, 24,  148, 248, 24,  152, 245, 24,  157, 243, 22,
        161, 241, 22,  162, 238, 22,  164, 236, 22,  164, 233, 21,  165, 231, 21,  167, 229, 21,
        167, 226, 21,  168, 224, 21,  169, 221, 20,  169, 219, 20,  170, 217, 20,  170, 214, 20,
        171, 212, 19,  172, 209, 19,  172, 207, 19,  173, 204, 19,  173, 202, 18,  172, 200, 18,
        173, 197, 18,  174, 195, 18,  173, 192, 17,  174, 190, 16,  173, 188, 16,  173, 185, 16,
        174, 183, 16,  174, 180, 16,  174, 178, 15,  175, 176, 15,  173, 172, 15,  171, 167, 15,
        168, 162, 14,  166, 159, 14,  164, 154, 14,  161, 149, 14,  159, 145, 14,  156, 140, 13,
        154, 136, 13,  152, 132, 13,  149, 128, 13,  147, 123, 12,  144, 119, 12,  142, 115, 11,
        140, 111, 11,  137, 107, 11,  135, 103, 11,  132, 99,  11,  130, 96,  10,  128, 92,  10,
        255, 181, 20,  255, 178, 20,  255, 174, 20,  255, 170, 20,  255, 167, 20,  255, 164, 20,
        255, 159, 20,  255, 155, 20,  255, 152, 20,  255, 148, 20,  255, 145, 19,  255, 142, 19,
        255, 136, 19,  255, 133, 19,  255, 130, 19,  255, 126, 19,  255, 123, 19,  255, 119, 19,
        255, 115, 19,  255, 111, 19,  255, 108, 19,  255, 105, 19,  255, 100, 19,  255, 96,  19,
        255, 93,  19,  255, 89,  19,  255, 85,  19,  255, 82,  19,  255, 79,  18,  255, 74,  18,
        255, 71,  18,  255, 67,  18,  255, 63,  18,  255, 60,  18,  255, 56,  18,  255, 52,  18,
        255, 48,  18,  255, 45,  18,  255, 41,  18,  255, 37,  18,
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 3;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255.);
    }
}

void ColorMaps::SetEField(vtkLookupTable *lut)
{
    unsigned char v[256 * 3] = {
        237, 253, 135, 233, 251, 137, 232, 251, 137, 230, 249, 140, 226, 249, 140, 225, 247, 142,
        223, 246, 144, 221, 246, 145, 218, 244, 147, 216, 244, 149, 214, 242, 149, 211, 242, 151,
        209, 240, 153, 207, 239, 154, 205, 239, 156, 202, 239, 158, 200, 237, 158, 198, 237, 161,
        195, 235, 161, 193, 235, 163, 191, 233, 165, 188, 232, 167, 186, 232, 168, 184, 230, 170,
        181, 230, 170, 179, 228, 172, 177, 228, 174, 175, 226, 175, 172, 226, 177, 170, 226, 179,
        168, 225, 179, 165, 223, 181, 163, 223, 182, 161, 221, 184, 158, 221, 186, 156, 219, 188,
        154, 219, 188, 151, 218, 191, 149, 216, 191, 147, 216, 193, 144, 214, 195, 142, 214, 195,
        140, 214, 198, 137, 212, 198, 135, 212, 200, 133, 211, 202, 130, 209, 204, 128, 209, 205,
        126, 207, 207, 124, 207, 207, 121, 205, 209, 119, 205, 211, 117, 204, 212, 114, 202, 214,
        112, 202, 216, 110, 202, 216, 107, 200, 219, 105, 200, 219, 103, 198, 221, 100, 198, 223,
        98,  196, 225, 96,  195, 226, 93,  195, 228, 91,  193, 228, 89,  193, 230, 86,  191, 232,
        84,  191, 233, 82,  189, 235, 80,  189, 237, 77,  188, 237, 75,  188, 239, 73,  186, 240,
        70,  186, 242, 68,  184, 244, 66,  184, 246, 65,  182, 246, 61,  181, 249, 59,  181, 249,
        58,  179, 251, 54,  179, 253, 52,  177, 255, 54,  175, 255, 54,  174, 255, 54,  172, 255,
        56,  170, 255, 56,  168, 255, 58,  167, 255, 58,  165, 255, 59,  163, 255, 59,  161, 255,
        61,  158, 255, 61,  156, 255, 61,  154, 255, 63,  154, 255, 63,  151, 255, 63,  149, 255,
        65,  147, 255, 66,  145, 255, 66,  144, 255, 66,  142, 255, 68,  140, 255, 68,  140, 255,
        70,  137, 255, 70,  135, 255, 70,  135, 255, 72,  133, 255, 73,  131, 255, 73,  130, 255,
        73,  128, 255, 75,  126, 255, 75,  124, 255, 75,  124, 255, 77,  121, 255, 77,  121, 255,
        79,  119, 255, 79,  117, 255, 79,  117, 255, 80,  114, 255, 82,  114, 255, 82,  112, 255,
        82,  110, 255, 84,  110, 255, 84,  107, 255, 84,  107, 255, 86,  105, 255, 86,  105, 255,
        87,  103, 255, 87,  102, 255, 89,  100, 255, 89,  100, 255, 91,  98,  255, 91,  96,  255,
        89,  94,  255, 89,  91,  255, 87,  89,  255, 89,  86,  255, 89,  86,  255, 89,  84,  255,
        91,  84,  255, 91,  82,  255, 93,  82,  255, 93,  80,  255, 94,  79,  255, 96,  79,  255,
        98,  77,  255, 98,  77,  255, 100, 75,  255, 100, 75,  255, 102, 73,  255, 103, 73,  255,
        105, 72,  255, 105, 70,  255, 107, 70,  255, 107, 68,  255, 110, 68,  255, 110, 66,  255,
        112, 65,  255, 114, 63,  255, 114, 63,  255, 117, 61,  255, 117, 61,  255, 119, 59,  255,
        121, 59,  255, 123, 58,  255, 124, 56,  255, 126, 56,  255, 128, 54,  255, 128, 54,  255,
        130, 52,  255, 131, 52,  253, 133, 52,  253, 135, 51,  253, 137, 49,  253, 137, 49,  253,
        138, 49,  253, 140, 47,  253, 142, 47,  253, 144, 47,  251, 144, 45,  251, 147, 45,  251,
        147, 45,  251, 149, 45,  251, 151, 43,  251, 153, 42,  251, 154, 42,  251, 156, 42,  251,
        156, 40,  249, 158, 40,  249, 160, 40,  249, 161, 38,  249, 163, 38,  249, 165, 38,  249,
        167, 36,  249, 168, 36,  249, 170, 35,  247, 170, 35,  247, 172, 35,  247, 175, 33,  247,
        175, 33,  246, 177, 33,  246, 179, 31,  246, 181, 31,  246, 182, 31,  246, 184, 29,  246,
        186, 29,  246, 188, 28,  246, 189, 28,  246, 191, 28,  244, 193, 26,  244, 195, 26,  244,
        195, 26,  244, 198, 26,  244, 200, 24,  244, 202, 24,  244, 202, 24,  244, 205, 22,  242,
        207, 22,  242, 209, 22,  242, 209, 21,  242, 212, 21,  242, 214, 19,  242, 216, 19,  242,
        218, 19,  242, 219, 17,  242, 221, 17,  240, 223, 17,  240, 225, 15,  240, 226, 15,  240,
        228, 15,  239, 230, 15,  239, 232, 14,  239, 233, 12,  239, 235, 12,  239, 237, 12,  239,
        239, 12,  237, 239, 10,  235, 239, 10,  232, 237, 10,  230, 237, 8,   228, 237, 8,   226,
        237, 8,   223, 237, 8,   221, 237, 7,   219, 237, 5,   216, 237, 5,   214, 235, 5,   212,
        235, 5,   209, 235, 3,   207, 235, 3,   204, 235, 3,   202, 235, 1,   200, 235, 1,   196,
        235, 1,   195, 235, 1,   191, 233, 0,   189, 233, 0,   186,
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 3;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255.);
    }
}

void ColorMaps::SetRun1(vtkLookupTable *lut)
{
    unsigned char v[256 * 4] = {
        0,   255, 0,   0,   64,  198, 255, 0,   128, 3,   255, 233, 192, 78,  47,  255, 1,   255,
        4,   0,   65,  193, 255, 0,   129, 4,   255, 238, 193, 80,  45,  255, 2,   255, 9,   0,
        66,  189, 255, 0,   130, 5,   255, 243, 194, 82,  43,  255, 3,   255, 14,  0,   67,  182,
        255, 0,   131, 6,   255, 248, 195, 85,  41,  255, 4,   255, 19,  0,   68,  175, 255, 0,
        132, 7,   255, 253, 196, 87,  39,  255, 5,   255, 24,  0,   69,  168, 255, 0,   133, 8,
        252, 255, 197, 90,  37,  255, 6,   255, 29,  0,   70,  161, 255, 0,   134, 9,   247, 255,
        198, 93,  35,  255, 7,   255, 34,  0,   71,  154, 255, 0,   135, 10,  242, 255, 199, 96,
        33,  255, 8,   255, 38,  0,   72,  147, 255, 0,   136, 10,  238, 255, 200, 98,  31,  255,
        9,   255, 43,  0,   73,  140, 255, 0,   137, 11,  233, 255, 201, 101, 29,  255, 10,  255,
        48,  0,   74,  133, 255, 0,   138, 12,  228, 255, 202, 104, 27,  255, 11,  255, 53,  0,
        75,  126, 255, 0,   139, 13,  224, 255, 203, 108, 25,  255, 12,  255, 58,  0,   76,  119,
        255, 0,   140, 14,  219, 255, 204, 111, 23,  255, 13,  255, 63,  0,   77,  112, 255, 0,
        141, 15,  215, 255, 205, 114, 21,  255, 14,  255, 68,  0,   78,  105, 255, 0,   142, 16,
        210, 255, 206, 117, 19,  255, 15,  255, 72,  0,   79,  98,  255, 0,   143, 17,  206, 255,
        207, 121, 17,  255, 16,  255, 77,  0,   80,  91,  255, 0,   144, 18,  202, 255, 208, 124,
        15,  255, 17,  255, 82,  0,   81,  84,  255, 0,   145, 19,  197, 255, 209, 128, 13,  255,
        18,  255, 87,  0,   82,  77,  255, 0,   146, 20,  193, 255, 210, 131, 11,  255, 19,  255,
        92,  0,   83,  70,  255, 0,   147, 20,  189, 255, 211, 135, 9,   255, 20,  255, 97,  0,
        84,  64,  255, 0,   148, 21,  185, 255, 212, 139, 7,   255, 21,  255, 102, 0,   85,  57,
        255, 0,   149, 22,  180, 255, 213, 143, 5,   255, 22,  255, 106, 0,   86,  50,  255, 0,
        150, 23,  176, 255, 214, 147, 3,   255, 23,  255, 111, 0,   87,  43,  255, 0,   151, 24,
        172, 255, 215, 151, 1,   255, 24,  255, 116, 0,   88,  36,  255, 0,   152, 25,  168, 255,
        216, 155, 0,   255, 25,  255, 121, 0,   89,  29,  255, 0,   153, 26,  164, 255, 217, 159,
        0,   255, 26,  255, 126, 0,   90,  22,  255, 0,   154, 27,  160, 255, 218, 163, 0,   255,
        27,  255, 131, 0,   91,  15,  255, 0,   155, 28,  156, 255, 219, 167, 0,   255, 28,  255,
        136, 0,   92,  8,   255, 0,   156, 29,  152, 255, 220, 172, 0,   255, 29,  255, 141, 0,
        93,  1,   255, 0,   157, 30,  148, 255, 221, 176, 0,   255, 30,  255, 145, 0,   94,  0,
        255, 5,   158, 30,  144, 255, 222, 180, 0,   255, 31,  255, 150, 0,   95,  0,   255, 12,
        159, 31,  140, 255, 223, 184, 0,   255, 32,  255, 155, 0,   96,  0,   255, 19,  160, 32,
        136, 255, 224, 188, 0,   255, 33,  255, 160, 0,   97,  0,   255, 26,  161, 33,  133, 255,
        225, 192, 0,   255, 34,  255, 165, 0,   98,  0,   255, 33,  162, 34,  129, 255, 226, 197,
        0,   255, 35,  255, 170, 0,   99,  0,   255, 40,  163, 35,  125, 255, 227, 201, 0,   255,
        36,  255, 175, 0,   100, 0,   255, 47,  164, 36,  122, 255, 228, 205, 0,   255, 37,  255,
        179, 0,   101, 0,   255, 54,  165, 37,  118, 255, 229, 209, 0,   255, 38,  255, 184, 0,
        102, 0,   255, 60,  166, 38,  114, 255, 230, 213, 0,   255, 39,  255, 189, 0,   103, 0,
        255, 67,  167, 39,  111, 255, 231, 217, 0,   255, 40,  255, 194, 0,   104, 0,   255, 74,
        168, 40,  107, 255, 232, 222, 0,   255, 41,  255, 199, 0,   105, 0,   255, 81,  169, 40,
        104, 255, 233, 226, 0,   255, 42,  255, 204, 0,   106, 0,   255, 88,  170, 41,  100, 255,
        234, 230, 0,   255, 43,  255, 209, 0,   107, 0,   255, 95,  171, 42,  97,  255, 235, 234,
        0,   255, 44,  255, 213, 0,   108, 0,   255, 102, 172, 43,  93,  255, 236, 238, 0,   255,
        45,  255, 218, 0,   109, 0,   255, 109, 173, 44,  90,  255, 237, 242, 0,   255, 46,  255,
        223, 0,   110, 0,   255, 116, 174, 45,  87,  255, 238, 247, 0,   255, 47,  255, 228, 0,
        111, 0,   255, 123, 175, 46,  84,  255, 239, 251, 0,   255, 48,  255, 233, 0,   112, 0,
        255, 130, 176, 47,  80,  255, 240, 255, 0,   254, 49,  255, 238, 0,   113, 0,   255, 137,
        177, 48,  77,  255, 241, 255, 0,   250, 50,  255, 243, 0,   114, 0,   255, 144, 178, 49,
        74,  255, 242, 255, 0,   246, 51,  255, 248, 0,   115, 0,   255, 151, 179, 50,  71,  255,
        243, 255, 0,   242, 52,  255, 252, 0,   116, 0,   255, 158, 180, 50,  68,  255, 244, 255,
        0,   237, 53,  252, 255, 0,   117, 0,   255, 165, 181, 53,  66,  255, 245, 255, 0,   233,
        54,  247, 255, 0,   118, 0,   255, 172, 182, 55,  64,  255, 246, 255, 0,   229, 55,  242,
        255, 0,   119, 0,   255, 178, 183, 57,  62,  255, 247, 255, 0,   225, 56,  237, 255, 0,
        120, 0,   255, 185, 184, 59,  60,  255, 248, 255, 0,   221, 57,  232, 255, 0,   121, 0,
        255, 192, 185, 63,  61,  255, 249, 255, 0,   217, 58,  227, 255, 0,   122, 0,   255, 199,
        186, 65,  59,  255, 250, 255, 0,   212, 59,  223, 255, 0,   123, 0,   255, 206, 187, 67,
        57,  255, 251, 255, 0,   208, 60,  218, 255, 0,   124, 0,   255, 213, 188, 69,  55,  255,
        252, 255, 0,   204, 61,  213, 255, 0,   125, 0,   255, 218, 189, 71,  53,  255, 253, 255,
        0,   200, 62,  208, 255, 0,   126, 1,   255, 223, 190, 73,  51,  255, 254, 255, 0,   196,
        63,  203, 255, 0,   127, 2,   255, 228, 191, 75,  49,  255, 255, 255, 0,   191
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 4;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255., v[k + 3] / 255.);
    }
}

void ColorMaps::SetRun2(vtkLookupTable *lut)
{
    unsigned char v[256 * 4] = {
        0,   255, 89,  89,  64,  255, 151, 151, 128, 255, 255, 255, 192, 130, 142, 255, 1,   255,
        90,  90,  65,  255, 153, 153, 129, 253, 253, 255, 193, 128, 141, 255, 2,   255, 91,  91,
        66,  255, 154, 154, 130, 251, 251, 255, 194, 127, 140, 255, 3,   255, 92,  92,  67,  255,
        156, 156, 131, 249, 249, 255, 195, 126, 139, 255, 4,   255, 93,  93,  68,  255, 157, 157,
        132, 247, 247, 255, 196, 125, 138, 255, 5,   255, 94,  94,  69,  255, 159, 159, 133, 245,
        246, 255, 197, 124, 137, 255, 6,   255, 95,  95,  70,  255, 161, 161, 134, 243, 244, 255,
        198, 123, 136, 255, 7,   255, 96,  96,  71,  255, 162, 162, 135, 241, 242, 255, 199, 121,
        135, 255, 8,   255, 97,  97,  72,  255, 164, 164, 136, 239, 240, 255, 200, 120, 134, 255,
        9,   255, 97,  97,  73,  255, 165, 165, 137, 237, 239, 255, 201, 119, 133, 255, 10,  255,
        98,  98,  74,  255, 167, 167, 138, 235, 237, 255, 202, 118, 132, 255, 11,  255, 99,  99,
        75,  255, 169, 169, 139, 233, 235, 255, 203, 117, 130, 255, 12,  255, 100, 100, 76,  255,
        170, 170, 140, 231, 233, 255, 204, 116, 129, 255, 13,  255, 101, 101, 77,  255, 172, 172,
        141, 229, 232, 255, 205, 114, 128, 255, 14,  255, 102, 102, 78,  255, 174, 174, 142, 227,
        230, 255, 206, 113, 127, 255, 15,  255, 103, 103, 79,  255, 175, 175, 143, 225, 228, 255,
        207, 112, 126, 255, 16,  255, 104, 104, 80,  255, 177, 177, 144, 223, 226, 255, 208, 111,
        125, 255, 17,  255, 105, 105, 81,  255, 178, 178, 145, 221, 224, 255, 209, 110, 124, 255,
        18,  255, 106, 106, 82,  255, 180, 180, 146, 219, 223, 255, 210, 108, 123, 255, 19,  255,
        107, 107, 83,  255, 182, 182, 147, 217, 221, 255, 211, 107, 122, 255, 20,  255, 108, 108,
        84,  255, 183, 183, 148, 215, 219, 255, 212, 106, 121, 255, 21,  255, 109, 109, 85,  255,
        185, 185, 149, 213, 217, 255, 213, 105, 120, 255, 22,  255, 110, 110, 86,  255, 187, 187,
        150, 211, 216, 255, 214, 104, 119, 255, 23,  255, 111, 111, 87,  255, 188, 188, 151, 209,
        214, 255, 215, 103, 118, 255, 24,  255, 112, 112, 88,  255, 190, 190, 152, 207, 212, 255,
        216, 101, 117, 255, 25,  255, 113, 113, 89,  255, 191, 191, 153, 205, 210, 255, 217, 100,
        116, 255, 26,  255, 114, 114, 90,  255, 193, 193, 154, 203, 209, 255, 218, 99,  115, 255,
        27,  255, 115, 115, 91,  255, 195, 195, 155, 201, 207, 255, 219, 98,  114, 255, 28,  255,
        116, 116, 92,  255, 196, 196, 156, 200, 205, 255, 220, 97,  112, 255, 29,  255, 117, 117,
        93,  255, 198, 198, 157, 198, 203, 255, 221, 96,  111, 255, 30,  255, 118, 118, 94,  255,
        199, 199, 158, 196, 201, 255, 222, 94,  110, 255, 31,  255, 119, 119, 95,  255, 201, 201,
        159, 194, 200, 255, 223, 93,  109, 255, 32,  255, 120, 120, 96,  255, 203, 203, 160, 192,
        198, 255, 224, 92,  108, 255, 33,  255, 121, 121, 97,  255, 204, 204, 161, 190, 196, 255,
        225, 91,  107, 255, 34,  255, 122, 122, 98,  255, 206, 206, 162, 188, 194, 255, 226, 90,
        106, 255, 35,  255, 123, 123, 99,  255, 208, 208, 163, 186, 193, 255, 227, 88,  105, 255,
        36,  255, 124, 124, 100, 255, 209, 209, 164, 184, 191, 255, 228, 87,  104, 255, 37,  255,
        125, 125, 101, 255, 211, 211, 165, 182, 189, 255, 229, 86,  103, 255, 38,  255, 126, 126,
        102, 255, 212, 212, 166, 180, 187, 255, 230, 85,  102, 255, 39,  255, 127, 127, 103, 255,
        214, 214, 167, 178, 186, 255, 231, 84,  101, 255, 40,  255, 128, 128, 104, 255, 216, 216,
        168, 176, 184, 255, 232, 83,  100, 255, 41,  255, 129, 129, 105, 255, 217, 217, 169, 174,
        182, 255, 233, 81,  99,  255, 42,  255, 130, 130, 106, 255, 219, 219, 170, 172, 180, 255,
        234, 80,  98,  255, 43,  255, 131, 131, 107, 255, 221, 221, 171, 170, 178, 255, 235, 79,
        97,  255, 44,  255, 131, 131, 108, 255, 222, 222, 172, 168, 177, 255, 236, 78,  96,  255,
        45,  255, 132, 132, 109, 255, 224, 224, 173, 166, 175, 255, 237, 77,  95,  255, 46,  255,
        133, 133, 110, 255, 225, 225, 174, 164, 173, 255, 238, 76,  93,  255, 47,  255, 134, 134,
        111, 255, 227, 227, 175, 162, 171, 255, 239, 74,  92,  255, 48,  255, 135, 135, 112, 255,
        229, 229, 176, 160, 170, 255, 240, 73,  91,  255, 49,  255, 136, 136, 113, 255, 230, 230,
        177, 158, 168, 255, 241, 72,  90,  255, 50,  255, 137, 137, 114, 255, 232, 232, 178, 156,
        166, 255, 242, 71,  89,  255, 51,  255, 138, 138, 115, 255, 233, 233, 179, 154, 164, 255,
        243, 70,  88,  255, 52,  255, 139, 139, 116, 255, 235, 235, 180, 152, 163, 255, 244, 69,
        87,  255, 53,  255, 140, 140, 117, 255, 237, 237, 181, 150, 161, 255, 245, 67,  86,  255,
        54,  255, 141, 141, 118, 255, 238, 238, 182, 148, 159, 255, 246, 66,  85,  255, 55,  255,
        142, 142, 119, 255, 240, 240, 183, 146, 157, 255, 247, 65,  84,  255, 56,  255, 143, 143,
        120, 255, 242, 242, 184, 145, 156, 255, 248, 64,  83,  255, 57,  255, 144, 144, 121, 255,
        243, 243, 185, 143, 154, 255, 249, 63,  82,  255, 58,  255, 145, 145, 122, 255, 245, 245,
        186, 141, 152, 255, 250, 61,  81,  255, 59,  255, 146, 146, 123, 255, 246, 246, 187, 139,
        150, 255, 251, 60,  80,  255, 60,  255, 147, 147, 124, 255, 248, 248, 188, 137, 148, 255,
        252, 59,  79,  255, 61,  255, 148, 148, 125, 255, 250, 250, 189, 135, 147, 255, 253, 58,
        78,  255, 62,  255, 149, 149, 126, 255, 251, 251, 190, 133, 145, 255, 254, 57,  77,  255,
        63,  255, 150, 150, 127, 255, 253, 253, 191, 131, 143, 255, 255, 56,  75,  255
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 4;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255., v[k + 3] / 255.);
    }
}

void ColorMaps::SetVolRenGreen(vtkLookupTable *lut)
{
    unsigned char v[256 * 4] = {
        0,   0,   255, 0,   0,   1,   253, 0,   0,   3,   252, 1,   0,   5,   251, 1,   0,   7,
        249, 2,   0,   9,   248, 3,   0,   11,  247, 3,   0,   13,  245, 4,   0,   14,  244, 5,
        0,   16,  243, 5,   0,   18,  241, 6,   0,   20,  240, 6,   0,   22,  239, 7,   0,   24,
        237, 8,   0,   26,  236, 8,   0,   28,  235, 9,   0,   29,  233, 10,  0,   31,  232, 10,
        0,   33,  231, 11,  0,   35,  229, 11,  0,   37,  228, 12,  0,   39,  227, 13,  0,   41,
        225, 13,  0,   43,  224, 14,  0,   44,  223, 15,  0,   46,  221, 15,  0,   48,  220, 16,
        0,   50,  219, 16,  0,   52,  217, 17,  0,   54,  216, 18,  0,   56,  215, 18,  0,   58,
        213, 19,  0,   59,  212, 20,  0,   61,  211, 20,  0,   63,  209, 21,  0,   65,  208, 21,
        0,   67,  207, 22,  0,   69,  206, 23,  0,   71,  204, 23,  0,   73,  203, 24,  0,   75,
        202, 25,  0,   76,  200, 25,  0,   78,  199, 26,  0,   80,  198, 26,  0,   82,  196, 27,
        0,   84,  195, 28,  0,   86,  194, 28,  0,   88,  192, 29,  0,   89,  191, 30,  0,   91,
        190, 30,  0,   93,  188, 31,  0,   95,  187, 31,  0,   97,  186, 32,  0,   99,  184, 33,
        0,   101, 183, 33,  0,   103, 182, 34,  0,   105, 180, 35,  0,   106, 179, 35,  0,   108,
        178, 36,  0,   110, 176, 36,  0,   112, 175, 37,  0,   114, 174, 38,  0,   116, 172, 38,
        0,   118, 171, 39,  0,   119, 170, 40,  0,   121, 168, 40,  0,   123, 167, 41,  0,   125,
        166, 41,  0,   127, 164, 42,  0,   129, 163, 43,  0,   131, 162, 43,  0,   133, 160, 44,
        0,   135, 159, 45,  0,   136, 158, 45,  0,   138, 157, 46,  0,   140, 155, 46,  0,   142,
        154, 47,  0,   144, 153, 48,  0,   146, 151, 48,  0,   148, 150, 49,  0,   149, 149, 50,
        0,   151, 147, 50,  0,   153, 146, 51,  0,   155, 145, 51,  0,   157, 143, 52,  0,   159,
        142, 53,  0,   161, 141, 53,  0,   163, 139, 54,  0,   165, 138, 55,  0,   166, 137, 55,
        0,   168, 135, 56,  0,   170, 134, 56,  0,   172, 133, 57,  0,   174, 131, 58,  0,   176,
        130, 58,  0,   178, 129, 59,  0,   179, 127, 60,  0,   181, 126, 60,  0,   183, 125, 61,
        0,   185, 123, 61,  0,   187, 122, 62,  0,   189, 121, 63,  0,   191, 119, 63,  0,   193,
        118, 64,  0,   195, 117, 65,  0,   196, 115, 65,  0,   198, 114, 66,  0,   200, 113, 66,
        0,   202, 111, 67,  0,   204, 110, 68,  0,   206, 109, 68,  0,   208, 108, 69,  0,   210,
        106, 70,  0,   211, 105, 70,  0,   213, 104, 71,  0,   215, 102, 71,  0,   217, 101, 72,
        0,   219, 100, 73,  0,   221, 98,  73,  0,   223, 97,  74,  0,   225, 96,  75,  0,   226,
        94,  75,  0,   228, 93,  76,  0,   230, 92,  76,  0,   232, 90,  77,  0,   234, 89,  78,
        0,   236, 88,  78,  0,   238, 86,  79,  0,   239, 85,  80,  0,   241, 84,  80,  0,   243,
        82,  81,  0,   245, 81,  81,  0,   247, 80,  82,  0,   249, 78,  83,  0,   251, 77,  83,
        0,   253, 76,  84,  0,   255, 74,  85,  1,   254, 75,  85,  3,   255, 76,  86,  5,   254,
        76,  86,  7,   255, 77,  87,  9,   255, 78,  88,  11,  255, 78,  88,  13,  255, 79,  89,
        15,  255, 80,  90,  17,  255, 80,  90,  19,  255, 81,  91,  21,  255, 82,  91,  22,  255,
        82,  92,  24,  255, 83,  93,  26,  255, 84,  93,  28,  255, 84,  94,  30,  255, 85,  95,
        32,  255, 86,  95,  34,  255, 86,  96,  36,  255, 87,  96,  38,  255, 88,  97,  40,  255,
        88,  98,  42,  255, 89,  98,  43,  255, 90,  99,  45,  255, 90,  100, 47,  255, 91,  100,
        49,  255, 92,  101, 51,  255, 92,  101, 53,  255, 93,  102, 55,  255, 94,  103, 57,  254,
        94,  103, 59,  255, 95,  104, 61,  254, 96,  105, 63,  255, 96,  105, 65,  255, 97,  106,
        66,  255, 98,  106, 68,  255, 98,  107, 70,  255, 99,  108, 72,  255, 100, 108, 74,  255,
        100, 109, 76,  255, 101, 110, 78,  255, 102, 110, 80,  255, 102, 111, 82,  255, 103, 111,
        84,  255, 104, 112, 86,  255, 104, 113, 87,  255, 105, 113, 89,  255, 106, 114, 91,  255,
        106, 115, 93,  255, 107, 115, 95,  255, 108, 116, 97,  255, 108, 117, 99,  254, 109, 117,
        101, 255, 110, 118, 103, 254, 110, 118, 105, 255, 111, 119, 107, 255, 112, 120, 109, 255,
        112, 120, 110, 255, 113, 121, 112, 255, 114, 122, 114, 255, 114, 122, 116, 255, 115, 123,
        118, 255, 116, 123, 120, 255, 116, 124, 122, 255, 117, 125, 124, 255, 118, 125, 126, 255,
        118, 126, 128, 255, 119, 127, 130, 255, 120, 127, 131, 255, 120, 128, 133, 255, 121, 128,
        135, 254, 122, 129, 137, 255, 122, 130, 139, 254, 123, 130, 141, 255, 124, 131, 143, 255,
        124, 132, 145, 255, 125, 132, 147, 255, 126, 133, 149, 255, 126, 133, 151, 255, 127, 134,
        153, 255, 128, 135, 154, 255, 128, 135, 156, 255, 129, 136, 158, 255, 130, 137, 160, 255,
        130, 137, 162, 255, 131, 138, 164, 255, 132, 138, 166, 255, 132, 139, 168, 255, 133, 140,
        170, 255, 134, 140, 172, 254, 134, 141, 174, 255, 135, 142, 175, 255, 136, 142, 177, 255,
        136, 143, 179, 254, 137, 143, 181, 255, 138, 144, 183, 255, 138, 145, 185, 255, 139, 145,
        187, 255, 140, 146, 189, 255, 140, 147, 191, 255, 141, 147, 193, 255, 142, 148, 195, 255,
        142, 148, 197, 254, 143, 149, 198, 255, 144, 150, 200, 254, 144, 150, 202, 255, 145, 151,
        204, 255, 146, 152, 206, 255, 146, 152, 208, 255, 147, 153, 210, 255, 148, 153, 212, 255,
        148, 154, 214, 255, 149, 155, 216, 254, 150, 155, 218, 255, 150, 156, 219, 255, 151, 157,
        221, 255, 152, 157, 223, 255, 152, 158, 225, 255, 153, 158, 227, 255, 154, 159
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 4;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255., v[k + 3] / 255.);
    }
}

void ColorMaps::SetVolRenGlow(vtkLookupTable *lut)
{
    unsigned char v[256 * 4] = {
        0,   0,   0,   0,   0,   0,   0,   1,   5,   0,   0,   2,   5,   0,   0,   3,   9,   0,
        0,   4,   9,   0,   0,   5,   13,  0,   0,   6,   13,  0,   5,   7,   17,  0,   5,   8,
        17,  0,   5,   9,   21,  0,   5,   10,  21,  0,   5,   11,  25,  0,   5,   12,  25,  0,
        5,   13,  29,  0,   5,   14,  29,  0,   9,   15,  33,  0,   9,   16,  33,  0,   9,   17,
        37,  0,   9,   18,  37,  0,   9,   19,  41,  0,   9,   20,  41,  0,   9,   21,  45,  0,
        9,   22,  45,  0,   9,   23,  49,  0,   13,  24,  49,  0,   13,  25,  53,  0,   13,  26,
        53,  0,   13,  27,  57,  0,   13,  28,  57,  0,   13,  29,  61,  0,   13,  30,  61,  5,
        13,  31,  65,  5,   13,  32,  65,  5,   13,  33,  69,  5,   13,  34,  69,  5,   13,  35,
        73,  5,   17,  36,  73,  5,   17,  37,  77,  5,   17,  38,  81,  5,   17,  39,  81,  5,
        17,  40,  85,  5,   17,  41,  85,  5,   17,  42,  89,  5,   17,  43,  89,  9,   17,  44,
        93,  9,   17,  45,  93,  9,   17,  46,  97,  9,   17,  47,  97,  9,   17,  48,  101, 9,
        17,  49,  101, 9,   17,  50,  105, 9,   17,  51,  105, 9,   17,  52,  109, 13,  17,  53,
        109, 13,  17,  54,  113, 13,  17,  55,  113, 13,  17,  56,  117, 13,  17,  57,  117, 13,
        17,  58,  121, 13,  17,  59,  125, 13,  17,  60,  125, 13,  17,  61,  128, 17,  17,  62,
        128, 17,  17,  63,  132, 17,  21,  64,  132, 17,  21,  65,  136, 17,  21,  66,  136, 17,
        21,  67,  140, 17,  21,  68,  140, 21,  21,  69,  144, 21,  21,  70,  144, 21,  21,  71,
        148, 21,  21,  72,  148, 21,  21,  73,  152, 25,  21,  74,  152, 25,  21,  75,  156, 25,
        21,  76,  156, 25,  21,  77,  156, 25,  21,  78,  160, 29,  21,  79,  160, 29,  21,  80,
        164, 29,  21,  81,  164, 29,  21,  82,  168, 33,  21,  83,  168, 33,  21,  84,  168, 33,
        21,  85,  172, 33,  21,  86,  172, 37,  21,  87,  176, 37,  21,  88,  176, 37,  21,  89,
        176, 37,  21,  90,  180, 41,  21,  91,  180, 41,  21,  92,  180, 41,  21,  93,  184, 45,
        21,  94,  184, 45,  21,  95,  188, 45,  21,  96,  188, 49,  21,  97,  188, 49,  21,  98,
        192, 49,  21,  99,  192, 53,  21,  100, 192, 53,  21,  101, 192, 53,  21,  102, 196, 57,
        21,  103, 196, 57,  21,  104, 196, 61,  21,  105, 200, 61,  21,  106, 200, 61,  21,  107,
        200, 65,  21,  108, 200, 65,  21,  109, 204, 69,  21,  110, 204, 69,  21,  111, 204, 69,
        21,  112, 208, 73,  21,  113, 208, 73,  21,  114, 208, 77,  21,  115, 208, 77,  21,  116,
        208, 81,  21,  117, 212, 81,  21,  118, 212, 85,  21,  119, 212, 85,  21,  120, 212, 89,
        21,  121, 212, 89,  21,  122, 216, 93,  21,  123, 216, 93,  21,  124, 216, 97,  21,  125,
        216, 97,  21,  126, 216, 101, 21,  127, 220, 101, 21,  128, 220, 105, 21,  129, 220, 105,
        21,  130, 220, 109, 21,  131, 220, 109, 21,  132, 220, 113, 21,  133, 220, 113, 21,  134,
        224, 117, 21,  135, 224, 117, 25,  136, 224, 121, 25,  137, 224, 121, 25,  138, 224, 125,
        25,  139, 224, 125, 25,  140, 224, 128, 25,  141, 228, 128, 25,  142, 228, 132, 25,  143,
        228, 132, 29,  144, 228, 136, 29,  145, 228, 136, 29,  146, 228, 140, 29,  147, 228, 140,
        29,  148, 228, 144, 33,  149, 232, 144, 33,  150, 232, 144, 33,  151, 232, 148, 33,  152,
        232, 148, 33,  153, 232, 152, 37,  154, 232, 152, 37,  155, 232, 156, 37,  156, 232, 156,
        37,  157, 236, 160, 41,  158, 236, 160, 41,  159, 236, 160, 41,  160, 236, 164, 41,  161,
        236, 164, 45,  162, 236, 168, 45,  163, 236, 168, 45,  164, 236, 172, 49,  165, 236, 172,
        49,  166, 236, 172, 49,  167, 240, 176, 53,  168, 240, 176, 53,  169, 240, 180, 53,  170,
        240, 180, 57,  171, 240, 180, 57,  172, 240, 184, 61,  173, 240, 184, 61,  174, 240, 188,
        61,  175, 240, 188, 65,  176, 240, 188, 65,  177, 240, 192, 69,  178, 244, 192, 69,  179,
        244, 192, 73,  180, 244, 192, 73,  181, 244, 192, 77,  182, 244, 200, 77,  183, 244, 200,
        81,  184, 244, 200, 81,  185, 244, 204, 85,  186, 244, 204, 85,  187, 244, 204, 89,  188,
        244, 208, 89,  189, 244, 208, 93,  190, 244, 208, 93,  191, 248, 212, 97,  192, 248, 212,
        97,  193, 248, 212, 101, 194, 248, 216, 105, 195, 248, 216, 105, 196, 248, 216, 109, 197,
        248, 220, 109, 198, 248, 220, 113, 199, 248, 220, 113, 200, 248, 224, 117, 201, 248, 224,
        121, 202, 248, 224, 121, 203, 248, 224, 125, 204, 248, 228, 125, 205, 248, 228, 128, 206,
        248, 228, 128, 207, 248, 228, 132, 208, 248, 232, 136, 209, 248, 232, 136, 210, 252, 232,
        140, 211, 252, 232, 144, 212, 252, 236, 144, 213, 252, 236, 148, 214, 252, 236, 148, 215,
        252, 236, 152, 216, 252, 240, 156, 217, 252, 240, 156, 218, 252, 240, 160, 219, 252, 240,
        160, 220, 252, 240, 164, 221, 252, 240, 168, 222, 252, 244, 168, 223, 252, 244, 172, 224,
        252, 244, 172, 225, 252, 244, 176, 226, 252, 244, 180, 227, 252, 244, 180, 228, 252, 248,
        184, 229, 252, 248, 188, 230, 252, 248, 188, 231, 252, 248, 192, 232, 252, 248, 192, 233,
        252, 248, 196, 234, 252, 248, 200, 235, 252, 248, 200, 236, 252, 252, 204, 237, 252, 252,
        208, 238, 252, 252, 208, 239, 252, 252, 212, 240, 252, 252, 212, 241, 252, 252, 216, 242,
        252, 252, 220, 243, 252, 252, 220, 244, 252, 252, 224, 245, 252, 252, 228, 246, 252, 252,
        228, 247, 252, 252, 232, 248, 252, 252, 236, 249, 252, 252, 236, 250, 252, 252, 240, 251,
        252, 252, 244, 252, 252, 252, 244, 253, 252, 252, 248, 254, 255, 255, 255, 255
    };

    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        vtkIdType k = i * 4;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255., v[k + 3] / 255.);
    }
}

void ColorMaps::SetVolRenRGB(vtkLookupTable *lut)
{
    unsigned char v[4 * 4] = {
        128, 128, 128, 0, 255, 0, 0, 50, 0, 255, 0, 100, 0, 0, 255, 150,
    };

    lut->SetNumberOfTableValues(4);
    for (vtkIdType i = 0; i < 4; ++i) {
        vtkIdType k = i * 4;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255., v[k + 3] / 255.);
    }
}

void ColorMaps::SetVolRenTwoLev(vtkLookupTable *lut)
{
    unsigned char v[8 * 4] = {
        128, 128, 128, 0, 128, 128, 128, 0,   255, 255, 0,   50, 128, 128, 128, 0,
        128, 128, 128, 0, 255, 50,  0,   100, 128, 128, 128, 0,  128, 128, 128, 0,
    };

    lut->SetNumberOfTableValues(8);
    for (vtkIdType i = 0; i < 8; ++i) {
        vtkIdType k = i * 4;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255., v[k + 3] / 255.);
    }
}

void ColorMaps::SetTenStep(vtkLookupTable *lut)
{
    unsigned char v[10 * 3] = {
        29, 0,   134, 0,   18,  163, 0,   74,  188, 0,   159, 195, 0,   201, 150,
        0,  209, 12,  141, 217, 0,   220, 221, 0,   226, 138, 0,   231, 48,  0,
    };

    lut->SetNumberOfTableValues(10);
    for (vtkIdType i = 0; i < 10; ++i) {
        vtkIdType k = i * 3;
        lut->SetTableValue(i, v[k] / 255., v[k + 1] / 255., v[k + 2] / 255.);
    }
}

void ColorMaps::SetPureRed(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, i / 255., 0., 0.);
    }
}

void ColorMaps::SetPureGreen(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 0., i / 255., 0.);
    }
}

void ColorMaps::SetPureBlue(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 0., 0., i / 255.);
    }
}

void ColorMaps::SetAllRed(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 1., 0., 0.);
    }
}

void ColorMaps::SetAllGreen(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 0., 1., 0.);
    }
}

void ColorMaps::SetAllBlue(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 0., 0., 1.);
    }
}

void ColorMaps::SetAllCyan(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 0., 1., 1.);
    }
}

void ColorMaps::SetAllMagenta(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 1., 0., 1.);
    }
}

void ColorMaps::SetAllYellow(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 1., 1., 0.);
    }
}

void ColorMaps::SetAllWhite(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 1., 1., 1.);
    }
}

void ColorMaps::SetAllBlack(vtkLookupTable *lut)
{
    lut->SetNumberOfTableValues(256);
    for (vtkIdType i = 0; i < 256; ++i) {
        lut->SetTableValue(i, 0., 0., 0.);
    }
}
