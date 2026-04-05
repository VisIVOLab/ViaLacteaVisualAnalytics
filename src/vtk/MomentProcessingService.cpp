#include "MomentProcessingService.h"

#include "app/BackendClient.h"

#include <QByteArray>
#include <QDebug>
#include <QThread>

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMomentMapFilter.h>

#include <cstring>

MomentProcessingService::MomentProcessingService(vtkMomentMapFilter *moment,
                                                 vtkLookupTable *lutMoment,
                                                 MomentComputeMode mode)
    : moment(moment), lutMoment(lutMoment), mode(mode)
{
}

MomentResult MomentProcessingService::computeMoment(const MomentRequest &request) const
{
    switch (this->mode) {
    case MomentComputeMode::Local:
        return this->computeMomentLocal(request);
    case MomentComputeMode::Remote:
        return this->computeMomentRemote(request);
    }

    return { nullptr, { 0., 0. }, false, QStringLiteral("Unsupported moment compute mode.") };
}

MomentResult MomentProcessingService::computeMomentLocal(const MomentRequest &request) const
{
    if (!this->moment || !this->lutMoment) {
        return { nullptr, { 0., 0. }, false, QStringLiteral("Local moment runtime not available.") };
    }

    switch (request.order) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 8:
    case 10:
        break;
    default:
        return { nullptr, { 0., 0. }, false, QStringLiteral("Unsupported moment order.") };
    }

    this->moment->SetMomentOrder(request.order);
    this->moment->Update();

    const double *range = this->moment->GetOutput()->GetScalarRange();
    this->lutMoment->SetTableRange(range);

    MomentResult result;
    result.valid = true;
    result.imageRange = { range[0], range[1] };
    result.image = vtkSmartPointer<vtkImageData>::New();
    result.image->DeepCopy(this->moment->GetOutput());
    return result;
}

MomentResult MomentProcessingService::computeMomentRemote(const MomentRequest &request) const
{
    if (request.datasetId.isEmpty()) {
        return { nullptr, { 0., 0. }, false, QStringLiteral("Missing remote dataset_id.") };
    }

    BackendClient client(request.backendUrl, request.backendToken);
    client.setSessionId(request.sessionId);
    BackendMomentResult response;
    bool usedTaskPath = false;

    const auto taskCreate = client.createMomentTask(request.datasetId, request.order, request.channelStart,
                                                    request.channelEnd, request.maskEnabled,
                                                    request.thresholdValue);
    if (taskCreate.valid && !taskCreate.taskId.isEmpty()) {
        usedTaskPath = true;
        qDebug().noquote()
                << QStringLiteral("[moment][task] created task_id=%1 status=%2 cache_hit=%3")
                           .arg(taskCreate.taskId, taskCreate.status)
                           .arg(taskCreate.cacheHit ? QStringLiteral("true")
                                                    : QStringLiteral("false"));
        constexpr int maxPollAttempts = 120;
        constexpr int pollIntervalMs = 250;
        for (int attempt = 0; attempt < maxPollAttempts; ++attempt) {
            QThread::msleep(pollIntervalMs);
            const auto taskStatus = client.requestTaskStatus(taskCreate.taskId);
            if (!taskStatus.valid && !taskStatus.status.isEmpty()
                && taskStatus.status != QStringLiteral("failed")) {
                qWarning().noquote()
                        << QStringLiteral("[moment][task] polling invalid task_id=%1 error=%2")
                                   .arg(taskCreate.taskId, taskStatus.error);
                break;
            }
            qDebug().noquote()
                    << QStringLiteral("[moment][task] poll task_id=%1 status=%2 progress=%3")
                               .arg(taskCreate.taskId, taskStatus.status)
                               .arg(taskStatus.progress, 0, 'f', 2);
            if (taskStatus.status == QStringLiteral("completed")) {
                response = BackendClient::parseMomentResultObject(taskStatus.resultObject);
                qDebug().noquote()
                        << QStringLiteral("[moment][task] completed task_id=%1 cache_hit=%2")
                                   .arg(taskCreate.taskId)
                                   .arg(taskStatus.cacheHit ? QStringLiteral("true")
                                                            : QStringLiteral("false"));
                break;
            }
            if (taskStatus.status == QStringLiteral("failed")) {
                qWarning().noquote()
                        << QStringLiteral("[moment][task] failed task_id=%1 error=%2")
                                   .arg(taskCreate.taskId, taskStatus.error);
                break;
            }
        }
        if (!response.valid) {
            qWarning().noquote()
                    << QStringLiteral("[moment][task] fallback to sync task_id=%1")
                               .arg(taskCreate.taskId);
        }
    } else {
        qWarning().noquote()
                << QStringLiteral("[moment][task] create failed, fallback to sync error=%1")
                           .arg(taskCreate.error);
    }

    if (!response.valid) {
        response = client.requestMoment(request.datasetId, request.order, request.channelStart,
                                        request.channelEnd, request.maskEnabled,
                                        request.thresholdValue);
        if (usedTaskPath) {
            qDebug().noquote() << QStringLiteral("[moment][task] sync fallback completed valid=%1")
                                          .arg(response.valid ? QStringLiteral("true")
                                                              : QStringLiteral("false"));
        }
    }

    if (!response.valid) {
        return { nullptr, { 0., 0. }, false,
                 response.error.isEmpty() ? QStringLiteral("Remote moment request failed.")
                                          : response.error };
    }

    if (response.scalarType != QStringLiteral("float32")) {
        return { nullptr, { 0., 0. }, false, QStringLiteral("Unsupported remote scalar type.") };
    }

    const qsizetype expectedBytes =
            static_cast<qsizetype>(response.width) * response.height * static_cast<qsizetype>(sizeof(float));
    if (response.width <= 0 || response.height <= 0 || response.data.size() != expectedBytes) {
        return { nullptr, { 0., 0. }, false, QStringLiteral("Invalid remote moment payload.") };
    }

    MomentResult result;
    result.valid = true;
    result.imageRange = { response.rangeMin, response.rangeMax };
    result.image = vtkSmartPointer<vtkImageData>::New();
    result.image->SetExtent(0, response.width - 1, 0, response.height - 1, 0, 0);
    result.image->AllocateScalars(VTK_FLOAT, 1);
    std::memcpy(result.image->GetScalarPointer(), response.data.constData(),
                static_cast<std::size_t>(expectedBytes));
    return result;
}

MomentMapResult MomentProcessingService::process(const MomentMapRequest &request) const
{
    const auto result = this->computeMoment(MomentRequest { {}, {}, {}, {}, {}, request.momentOrder });
    return { result.valid, result.imageRange };
}
