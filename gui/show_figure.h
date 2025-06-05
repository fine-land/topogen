#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QMessageBox>
#include <QPixmap>
#include <QScrollArea>

class FCTCDFConfigDialog : public QDialog
{
    Q_OBJECT
public:
    FCTCDFConfigDialog(QWidget *parent = nullptr);

private slots:
    void addFile();
    void removeFile();
    void generateChart();

private:
    QListWidget *fileListWidget;
    QLineEdit *titleEdit;
    QLineEdit *xRangeEdit;
    QLineEdit *yLabelEdit;
};

////////////         FCT_SLOWDOWN     ///////////////////////
class FctSlowDownDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FctSlowDownDialog(QWidget *parent = nullptr);
    QStringList getFilePaths() const;
    QStringList getAlgNames() const;
private slots:
    void addFile();

private:
    QVBoxLayout *fileLayout;
    QList<QLabel *> fileLabels;
    QList<QLineEdit *> algNameEdits;
    QStringList filePaths;
    QPushButton *addFileButton;
};

class ImageViewer : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewer(const QStringList &imagePaths, QWidget *parent = nullptr);
};


////////////////    THOUGHT_PUT    //////////////////////////
//class Thoughtput
//{
//  Q_OBJECT
//	public:
//};



class Thoughtput : public QDialog
{
    Q_OBJECT

public:
    explicit Thoughtput(QWidget *parent = nullptr);
    ~Thoughtput() = default;

private slots:
    void onFileSelectClicked();
    void generateChart(); // 假设需要生成图表

private:
    QString selectedFilePath;

    // UI components
    QLineEdit *filePathEdit; // 显示选择的文件路径
    QLabel *previewLabel; // 必须在这里声明 previewLabel 成员变量
    QLineEdit *startTimeEdit;
    QLineEdit *endTimeEdit;
};

