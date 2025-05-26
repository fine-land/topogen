#include "flowconfigdialog.h"
#include <QDebug>

// FlowConfigDialog 定义
FlowConfigDialog::FlowConfigDialog(const QString &trafficDir, QString &flowFilePath,
                                   QLabel *flowFileLabel, QLabel *statusLabel, QWidget *parent)
    : QDialog(parent), m_flowFilePath(flowFilePath), m_flowFileLabel(flowFileLabel), m_statusLabel(statusLabel)
{
    setWindowTitle("Configure Flow File Parameters");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGridLayout *gridLayout = new QGridLayout;

    qDebug() << "trafficDir is " << trafficDir << " flowFilePath " << flowFilePath
             << "\n";
    // 左侧描述
    QStringList labels = {"流文件:", "主机数量:", "负载:", "带宽:", "时间间隔:"};
    for (int i = 0; i < labels.size(); ++i)
    {
        gridLayout->addWidget(new QLabel(labels[i]), i, 0);
    }

    // 右侧输入控件
    // 流文件（下拉框）
    // m_configFileCombo = new QComboBox;
    // QDir dir(trafficDir);
    // dir.setNameFilters(QStringList() << "*.txt");
    // for (const QFileInfo &fileInfo : dir.entryInfoList())
    //{
    //    m_configFileCombo->addItem(fileInfo.fileName(), fileInfo.absoluteFilePath());
    //}
    // gridLayout->addWidget(m_configFileCombo, 0, 1);

    // 右侧输入控件
    // 流文件（下拉框）
    m_configFileCombo = new QComboBox;
    QStringList configFiles = {
        "AliStorage2019.txt",
        "GoogleRPC2008.txt",
        "WebSearch_distribution.txt",
        "FbHdp_distribution.txt"};
    for (const QString &fileName : configFiles)
    {
        // simulation/../
        QString filePath = trafficDir + "/" + fileName;
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists())
        {
            m_configFileCombo->addItem(fileName, filePath);
        }
        else
        {
            // 添加但禁用缺失的文件
            int index = m_configFileCombo->count();
            m_configFileCombo->addItem(fileName + " (missing)", filePath);
            m_configFileCombo->setItemData(index, QVariant(false), Qt::UserRole); // 标记为不可用
        }
    }
    gridLayout->addWidget(m_configFileCombo, 0, 1);

    // 主机数量
    m_numHostsEdit = new QLineEdit;
    m_numHostsEdit->setPlaceholderText("e.g., 3");
    gridLayout->addWidget(m_numHostsEdit, 1, 1);

    // 负载
    m_loadEdit = new QLineEdit;
    m_loadEdit->setPlaceholderText("e.g., 0.3");
    gridLayout->addWidget(m_loadEdit, 2, 1);

    // 带宽
    m_bandwidthEdit = new QLineEdit;
    m_bandwidthEdit->setPlaceholderText("e.g., 100G");
    gridLayout->addWidget(m_bandwidthEdit, 3, 1);

    // 时间间隔
    m_intervalEdit = new QLineEdit;
    m_intervalEdit->setPlaceholderText("e.g., 0.1");
    gridLayout->addWidget(m_intervalEdit, 4, 1);

    mainLayout->addLayout(gridLayout);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *generateButton = new QPushButton("Generate");
    QPushButton *cancelButton = new QPushButton("Cancel");
    buttonLayout->addWidget(generateButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 连接信号
    connect(generateButton, &QPushButton::clicked, this, &FlowConfigDialog::generateFlowFile);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void FlowConfigDialog::generateFlowFile()
{
    // 验证输入
    QString configFile = m_configFileCombo->currentData().toString();
    QString numHosts = m_numHostsEdit->text();
    QString load = m_loadEdit->text();
    QString bandwidth = m_bandwidthEdit->text();
    QString interval = m_intervalEdit->text();

    if (configFile.isEmpty() || numHosts.isEmpty() || load.isEmpty() || bandwidth.isEmpty() || interval.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please fill in all parameters!");
        return;
    }

    bool ok;
    int numHostsVal = numHosts.toInt(&ok);
    if (!ok || numHostsVal <= 0)
    {
        QMessageBox::warning(this, "Error", "Host number must be a positive integer!");
        return;
    }

    double loadVal = load.toDouble(&ok);
    if (!ok || loadVal <= 0)
    {
        QMessageBox::warning(this, "Error", "Load must be a positive number!");
        return;
    }

    QRegExp bandwidthRx("\\d+\\s*(G|M|K)?");
    if (!bandwidthRx.exactMatch(bandwidth))
    {
        QMessageBox::warning(this, "Error", "Bandwidth must be a number followed by G, M, or K (e.g., 100G)!");
        return;
    }

    double intervalVal = interval.toDouble(&ok);
    if (!ok || intervalVal <= 0)
    {
        QMessageBox::warning(this, "Error", "Interval must be a positive number!");
        return;
    }

    // 执行 Python 命令
    QProcess *process = new QProcess(this);
    process->setWorkingDirectory(QFileInfo(configFile).absolutePath()); // 设置为 ../traffic_gen/

    QString pythonCommand = "python";
    QStringList arguments;
    arguments << "traffic_gen.py" << "-c" << QFileInfo(configFile).fileName()
              << "-n" << numHosts << "-l" << load << "-b" << bandwidth << "-t" << interval;

    // 假设生成的文件名为 flow_<timestamp>.txt
    QString outputFileName = QString("flow_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString outputFilePath = QFileInfo(configFile).absolutePath() + "/" + outputFileName;

    QString command = QString("%1 %2").arg(pythonCommand, arguments.join(" "));
    qDebug() << "Executing command:" << command;
    process->start(pythonCommand, arguments);
    /*
    connect(process, &QProcess::readyReadStandardOutput, this, [=]()
            {
            QString output = process->readAllStandardOutput();
            QMessageBox::information(this, "Python Output", output); });

    connect(process, &QProcess::readyReadStandardError, this, [=]()
            {
            QString error = process->readAllStandardError();
            QMessageBox::warning(this, "Python Error", error); });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
            if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                // 更新主窗口
                m_flowFilePath = outputFilePath;
                m_flowFileLabel->setText(QFileInfo(outputFileName).baseName()); // 无 .txt
                m_statusLabel->setText("Flow File Generated");
                QMessageBox::information(this, "Success", "Flow file generated successfully!");
                accept(); // 关闭对话框
            } else {
                m_statusLabel->setText("Flow Generation Failed");
                QMessageBox::warning(this, "Error", QString("Flow generation failed with exit code: %1").arg(exitCode));
            }
            process->deleteLater(); });

    connect(process, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error)
            {
            QString errorMsg;
            switch (error) {
                case QProcess::FailedToStart:
                    errorMsg = "Failed to start Python process. Ensure Python is installed and traffic_gen.py exists.";
                    break;
                case QProcess::Crashed:
                    errorMsg = "Python process crashed.";
                    break;
                default:
                    errorMsg = "An error occurred while generating flow file.";
            }
            m_statusLabel->setText("Flow Generation Failed");
            QMessageBox::critical(this, "Process Error", errorMsg);
            process->deleteLater(); });

    if (!process->waitForStarted(3000))
    {
        m_statusLabel->setText("Flow Generation Failed");
        QMessageBox::critical(this, "Error", "Failed to start Python process within 3 seconds.");
        process->deleteLater();
    }
        */

    // 文件处理：复制并重命名 tmp_traffic.txt
    QString sourceFile = QFileInfo(configFile).absolutePath() + "/tmp_traffic.txt";
    QString configFileName = QFileInfo(configFile).baseName(); // 提取文件名，例如 AliStorage2019
    QString targetFileName = QString("%1_%2_%3_%4_%5.txt")
                                 .arg(configFileName, numHosts, load, bandwidth, interval);
    QString targetDir = QFileInfo(configFile).absolutePath() + "/../simulation/mix/";
    QString targetFile = targetDir + targetFileName;

    // 确保 mix/ 目录存在
    QDir mixDir(targetDir);
    if (!mixDir.exists())
    {
        mixDir.mkpath(".");
        qDebug() << "Created directory:" << targetDir;
    }

    // 检查源文件是否存在
    if (!QFile::exists(sourceFile))
    {
        qDebug() << "Source file does not exist:" << sourceFile;
        delete process;
        return;
    }

    // 复制并重命名文件
    if (QFile::copy(sourceFile, targetFile))
    {
        qDebug() << "File copied and renamed to:" << targetFile;
    }
    else
    {
        qDebug() << "Failed to copy file from" << sourceFile << "to" << targetFile;
    }

    // 清理
    delete process;
}
