#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDir>
#include <QProcess>
#include <QPushButton>
#include <QDateTime>

// FlowConfigDialog 定义
class FlowConfigDialog : public QDialog
{
    Q_OBJECT
public:
    FlowConfigDialog(const QString &trafficDir,
                     QString &flowFilePath, QLabel *flowFileLabel,
                     QLabel *statusLabel, QWidget *parent = nullptr);

private slots:
    void generateFlowFile();

private:
    QString &m_flowFilePath;
    QLabel *m_flowFileLabel;
    QLabel *m_statusLabel;
    QComboBox *m_configFileCombo;
    QLineEdit *m_numHostsEdit;
    QLineEdit *m_loadEdit;
    QLineEdit *m_bandwidthEdit;
    QLineEdit *m_intervalEdit;
};