#include "customprocess.h"
#include <QProcess>
#include <QDebug>
#include <QCoreApplication>

CustomProcess::CustomProcess()
{


    QProcess *process = new QProcess();
      connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished()));
      process->start("ls", QStringList());
      if (!process->waitForStarted())
          qDebug() << "error";



}

CustomProcess::~CustomProcess()
{

}

