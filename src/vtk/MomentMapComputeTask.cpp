#include "MomentMapComputeTask.h"

#include "MomentProcessingService.h"
#include "vtkFITSReader.h"
#include "vtkMomentMapFilter.h"

#include <QDebug>
#include <QElapsedTimer>

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>

MomentMapComputeResult computeMomentMap(const MomentMapComputeRequest &request)
{
    QElapsedTimer totalTimer;
    totalTimer.start();
    MomentMapComputeResult result;

    if (request.filepath.isEmpty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    vtkNew<vtkFITSReader> reader;
    reader->SetFileName(request.filepath.toUtf8());
    QElapsedTimer readTimer;
    readTimer.start();
    reader->Update();
    qDebug().noquote()
            << QStringLiteral("[perf][moment] FITS read: %1 ms").arg(readTimer.elapsed());

    vtkNew<vtkMomentMapFilter> moment;
    moment->SetInputConnection(reader->GetOutputPort());
    moment->Init(request.filepath.toStdString());

    vtkNew<vtkLookupTable> lutMoment;
    MomentProcessingService processing(moment, lutMoment);
    QElapsedTimer computeTimer;
    computeTimer.start();
    const auto processed = processing.process(MomentMapRequest { request.momentOrder });
    qDebug().noquote()
            << QStringLiteral("[perf][moment] worker compute: %1 ms").arg(computeTimer.elapsed());
    if (!processed.valid) {
        result.errorMessage = "Unsupported moment order.";
        return result;
    }

    result.valid = true;
    result.imageRange = processed.imageRange;
    result.imageData = vtkSmartPointer<vtkImageData>::New();
    QElapsedTimer deepCopyTimer;
    deepCopyTimer.start();
    result.imageData->DeepCopy(moment->GetOutput());
    qDebug().noquote()
            << QStringLiteral("[perf][moment] worker DeepCopy: %1 ms").arg(deepCopyTimer.elapsed());
    qDebug().noquote()
            << QStringLiteral("[perf][moment] worker total: %1 ms").arg(totalTimer.elapsed());
    return result;
}
