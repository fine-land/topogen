#include "show_figure.h"
#include <QAbstractItemView>
FCTCDFConfigDialog::FCTCDFConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("FCT CDF 配置");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 文件选择区域
    fileListWidget = new QListWidget(this);
    fileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    QPushButton *addFileButton = new QPushButton("添加文件", this);
    QPushButton *removeFileButton = new QPushButton("删除选中", this);
    QLabel *fileLabel = new QLabel("已选择的文件（1-5 个）：");
    QHBoxLayout *fileButtonLayout = new QHBoxLayout();
    fileButtonLayout->addWidget(addFileButton);
    fileButtonLayout->addWidget(removeFileButton);
    mainLayout->addWidget(fileLabel);
    mainLayout->addWidget(fileListWidget);
    mainLayout->addLayout(fileButtonLayout);

    // 参数输入
    // titleEdit = new QLineEdit("FCT 累积分布", this);
    // xRangeEdit = new QLineEdit("0,1000", this); // 默认 X 轴范围（0 到 1000 μs）
    // yLabelEdit = new QLineEdit("CDF (%)", this);
    // mainLayout->addWidget(new QLabel("图表标题:"));
    // mainLayout->addWidget(titleEdit);
    // mainLayout->addWidget(new QLabel("X 轴范围 (min,max, μs):"));
    // mainLayout->addWidget(xRangeEdit);
    // mainLayout->addWidget(new QLabel("Y 轴标签:"));
    // mainLayout->addWidget(yLabelEdit);

    // 确定按钮
    QPushButton *confirmButton = new QPushButton("确定", this);
    mainLayout->addWidget(confirmButton);

    connect(addFileButton, &QPushButton::clicked, this, &FCTCDFConfigDialog::addFile);
    connect(removeFileButton, &QPushButton::clicked, this, &FCTCDFConfigDialog::removeFile);
    connect(confirmButton, &QPushButton::clicked, this, &FCTCDFConfigDialog::generateChart);
}

void FCTCDFConfigDialog::addFile()
{
    if (fileListWidget->count() >= 5)
    {
        QMessageBox::warning(this, "警告", "最多选择 5 个文件！");
        return;
    }
    QString filePath = QFileDialog::getOpenFileName(this, "选择 FCT 文件", "mix/", "Text Files (*.txt);;CSV Files (*.csv)");
    if (!filePath.isEmpty())
    {
        // 避免重复添加
        for (int i = 0; i < fileListWidget->count(); ++i)
        {
            if (fileListWidget->item(i)->text() == filePath)
            {
                return;
            }
        }
        fileListWidget->addItem(filePath);
    }
}

void FCTCDFConfigDialog::removeFile()
{
    QList<QListWidgetItem *> selectedItems = fileListWidget->selectedItems();
    for (QListWidgetItem *item : selectedItems)
    {
        delete fileListWidget->takeItem(fileListWidget->row(item));
    }
}

void FCTCDFConfigDialog::generateChart()
{
    qDebug() << "生成图表按钮被点击！";
    if (fileListWidget->count() < 1)
    {
        QMessageBox::warning(this, "错误", "请至少选择 1 个 FCT 文件！");
        return;
    }

    QStringList filePaths;
    for (int i = 0; i < fileListWidget->count(); ++i)
    {
        filePaths << fileListWidget->item(i)->text();
        qDebug() << "Selected file:" << fileListWidget->item(i)->text();
    }

    // QString title = titleEdit->text();
    // QString xRange = xRangeEdit->text();
    // QString yLabel = yLabelEdit->text();

    // 调用 Python 脚本生成图表
    QProcess *process = new QProcess(this);
    QStringList arguments;
    arguments << "fct_cdf.py";

    for (int i = 0; i < filePaths.size() && i < 5; ++i) // Limit to 5 files as per Python script
    {
        arguments << QString("--file%1").arg(i + 1) << filePaths[i];
    }
    arguments << "--output" << "charts/fct_cdf";
    qDebug() << arguments;
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus)
            {
                    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                        // 显示图片
                        QPixmap pixmap("charts/fct_cdf.png");
                        if (!pixmap.isNull()) {
                            QLabel *imageLabel = new QLabel(this);
                            imageLabel->setPixmap(pixmap.scaled(800, 600, Qt::KeepAspectRatio));
                            QDialog *imageDialog = new QDialog(this);
                            QVBoxLayout *layout = new QVBoxLayout(imageDialog);
                            layout->addWidget(imageLabel);
                            imageDialog->setWindowTitle("FCT CDF 图");
                            imageDialog->resize(800, 600);
                            imageDialog->exec();
                        } else {
                            QMessageBox::warning(this, "错误", "无法加载生成的图片！");
                        }
                    } else {
                        QMessageBox::warning(this, "错误", QString("生成图表失败: %1").arg(QString(process->readAllStandardError())));
                    }
                    process->deleteLater(); });

    qDebug() << "执行命令: python3" << arguments.join(" ");
    process->start("python3", arguments);
    if (!process->waitForStarted())
    {
        QMessageBox::warning(this, "错误", "无法启动 Python3 脚本！");
        process->deleteLater();
    }
    else
    {
        accept(); // 关闭对话框
    }
}

FctSlowDownDialog::FctSlowDownDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("FCT Slow Down Configuration");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 文件选择区域
    fileLayout = new QVBoxLayout();
    fileLabels.clear();
    filePaths.clear();
    addFileButton = new QPushButton("添加文件", this);
    connect(addFileButton, &QPushButton::clicked, this, &FctSlowDownDialog::addFile);
    mainLayout->addLayout(fileLayout);
    mainLayout->addWidget(addFileButton);

    // 确定和取消按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", this);
    QPushButton *cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, &FctSlowDownDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &FctSlowDownDialog::reject);
}

QStringList FctSlowDownDialog::getFilePaths() const { return filePaths; }
QStringList FctSlowDownDialog::getAlgNames() const
{
    QStringList algNames;
    for (const auto &edit : algNameEdits)
    {
        algNames << edit->text().trimmed();
    }
    return algNames;
}

void FctSlowDownDialog::addFile()
{
    if (filePaths.size() >= 5)
    {
        QMessageBox::warning(this, "警告", "最多只能选择 5 个文件！");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, "选择 .txt 文件", "", "Text Files (*.txt)");
    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".txt"))
    {
        QMessageBox::warning(this, "错误", "请选择以 .txt 结尾的文件！");
        return;
    }

    filePaths << filePath;
    QHBoxLayout *fileRow = new QHBoxLayout();
    QLabel *fileLabel = new QLabel(filePath, this);
    QLineEdit *algNameEdit = new QLineEdit(this);
    algNameEdit->setPlaceholderText("输入算法名称");
    QPushButton *removeButton = new QPushButton("移除", this);

    fileLabels << fileLabel;
    algNameEdits << algNameEdit;

    fileRow->addWidget(fileLabel);
    fileRow->addWidget(algNameEdit);
    fileRow->addWidget(removeButton);
    fileLayout->addLayout(fileRow);

    connect(removeButton, &QPushButton::clicked, [this, fileRow, fileLabel, algNameEdit, removeButton]()
            {
            int index = fileLabels.indexOf(fileLabel);
            filePaths.removeAt(index);
            fileLabels.removeAt(index);
            algNameEdits.removeAt(index);
            fileLayout->removeItem(fileRow);
            delete fileLabel;
            delete algNameEdit;
            delete removeButton;
            delete fileRow; });
}

ImageViewer::ImageViewer(const QStringList &imagePaths, QWidget *parent) : QDialog(parent)
{
    qDebug() << "ImageViewer initialized with paths:" << imagePaths;
    setWindowTitle("FCT Slow Down Plots");
    QVBoxLayout *layout = new QVBoxLayout(this);
    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *container = new QWidget();
    QVBoxLayout *containerLayout = new QVBoxLayout(container);

    for (const QString &path : imagePaths)
    {
        qDebug() << "Processing image path:" << path;
        if (!(path.contains("avg") || path.contains("95") || path.contains("99")))
            continue;

        QPixmap pixmap(path);
        if (!pixmap.isNull())
        {
            QLabel *imageLabel = new QLabel(this);
            // imageLabel->setPixmap(pixmap);
            imageLabel->setPixmap(pixmap.scaled(800, 600, Qt::KeepAspectRatio));
            containerLayout->addWidget(imageLabel);
        }
    }

    container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    container->adjustSize();
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    setMinimumSize(850, 650);
}