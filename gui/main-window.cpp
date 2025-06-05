/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Strasbourg
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Pierre Weiss <3weissp@gmail.com>
 */

/**
 * \file main-window.cpp
 * \brief The main windows file.
 * \author Pierre Weiss
 * \date 2009
 */

#include <iostream>
#include <stdexcept>

#include "main-window.h"
#include "drag-widget.h"
#include "drag-object.h"

#include "ap.h"
#include "tap.h"
#include "emu.h"

#include "tcp-large-transfer.h"
#include "udp-echo.h"

#include "utils.h"
#include "gui-utils.h"
#include "array-utils.h"

#include "application-dialog.h"

#include "flowconfigdialog.h"
#include "show_figure.h"

MainWindow::MainWindow(const std::string &simulationName)
{
  progressDialog = new QProgressDialog{"Running simulation...", "Cancel", 0, 0, this};
  progressDialog->setWindowTitle("Simulation in Progress");
  progressDialog->setWindowModality(Qt::WindowModal); // 模态对话框，阻止用户操作主窗口
  progressDialog->setAutoClose(false);                // 手动控制关闭
  progressDialog->setAutoReset(false);
  // progressDialog->show(); // 显示进度对话框

  this->m_dw = nullptr;
  this->m_gen = new Generator(simulationName);

  //
  // 主布局
  //
  QVBoxLayout *mainLayout = new QVBoxLayout;

  //
  // 菜单
  //
  QMenu *menuFichier = menuBar()->addMenu("&File");
  QAction *menuSavePix = menuFichier->addAction("Save as picture");
  connect(menuSavePix, SIGNAL(triggered()), this, SLOT(SavePicture()));

  QAction *menuXml = menuFichier->addAction("Save as XML");
  connect(menuXml, SIGNAL(triggered()), this, SLOT(SaveXml()));

  QAction *menuTxt = menuFichier->addAction("Save as TXT");
  connect(menuTxt, SIGNAL(triggered()), this, SLOT(SaveTxt()));

  QAction *menuXmlLoad = menuFichier->addAction("Load XML file");
  connect(menuXmlLoad, SIGNAL(triggered()), this, SLOT(LoadXml()));

  QAction *actionQuit = menuFichier->addAction("Quit");
  connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

  QMenu *menuEdit = menuBar()->addMenu("&Show");
  QAction *actionFctCDF = menuEdit->addAction("FCT CDF");
  connect(actionFctCDF, &QAction::triggered, this, [=]()
          {
            FCTCDFConfigDialog *dialog = new FCTCDFConfigDialog(this);
            dialog->exec(); });
  QAction *actionFctSlowDown = menuEdit->addAction("FCT Slow Down");
  connect(actionFctSlowDown, &QAction::triggered, this, &MainWindow::onFctSlowDownTriggered);
  // actionConfig->setDisabled(true);
  
  QAction *actionThoughtPut = menuEdit->addAction("ThoutghtPut");
  connect(actionThoughtPut, &QAction::triggered, this, &MainWindow::ThoughtPut);


  QMenu *menuView = menuBar()->addMenu("&Generate");
  QAction *actionCpp = menuView->addAction("&C++");
  connect(actionCpp, SIGNAL(triggered()), this, SLOT(GenerateCpp()));
  QAction *actionPython = menuView->addAction("&Python");
  connect(actionPython, SIGNAL(triggered()), this, SLOT(GeneratePython()));

  QMenu *menuHelp = menuBar()->addMenu("&Help");
  QAction *menuOnlineHelp = menuHelp->addAction("Online Help");
  menuOnlineHelp->setDisabled(true);
  QAction *menuAbout = menuHelp->addAction("About");
  connect(menuAbout, SIGNAL(triggered()), this, SLOT(About()));

  QMenu *menuSimulation = menuBar()->addMenu("&Simulation");
  QAction *actionSelectTopoFile = menuSimulation->addAction("Select ns3 Topology file");
  connect(actionSelectTopoFile, SIGNAL(triggered()), this, SLOT(SelectTopoFile()));
  QAction *actionSelectFlowFile = menuSimulation->addAction("Select ns3 Flow file");
  connect(actionSelectFlowFile, SIGNAL(triggered()), this, SLOT(SelectFlowFile()));

  //
  // 第二行：工具栏
  //
  QToolBar *toolBarFichier = addToolBar("");
  QIcon pcIcon(":/Ico/Pc.png");
  QString pcString("Terminal");
  QAction *pcAction = toolBarFichier->addAction(pcIcon, pcString);
  connect(pcAction, SIGNAL(triggered()), this, SLOT(CreatePc()));

  QIcon pcgIcon(":/Ico/Pc-group.png");
  QString pcgString("Terminal Group");
  QAction *pcgAction = toolBarFichier->addAction(pcgIcon, pcgString);
  connect(pcgAction, SIGNAL(triggered()), this, SLOT(CreatePcGroup()));

  QIcon emuIcon(":/Ico/Emu.png");
  QString emuString("PC with emu");
  QAction *emuAction = toolBarFichier->addAction(emuIcon, emuString);
  connect(emuAction, SIGNAL(triggered()), this, SLOT(CreateEmu()));

  QIcon tapIcon(":/Ico/Tap.png");
  QString tapString("PC with tap");
  QAction *tapAction = toolBarFichier->addAction(tapIcon, tapString);
  connect(tapAction, SIGNAL(triggered()), this, SLOT(CreateTap()));

  QIcon apIcon(":/Ico/Ap-Wifi.png");
  QString apString("AP Wifi");
  QAction *apAction = toolBarFichier->addAction(apIcon, apString);
  connect(apAction, SIGNAL(triggered()), this, SLOT(CreateAp()));

  QIcon stasIcon(":/Ico/StationWifi.png");
  QString stasString("Station Wifi");
  QAction *stasAction = toolBarFichier->addAction(stasIcon, stasString);
  connect(stasAction, SIGNAL(triggered()), this, SLOT(CreateStation()));

  QIcon hubIcon(":/Ico/Hub.png");
  QString hubString("Hub");
  QAction *hubAction = toolBarFichier->addAction(hubIcon, hubString);
  connect(hubAction, SIGNAL(triggered()), this, SLOT(CreateHub()));

  QIcon switchIcon(":/Ico/Switch.png");
  QString switchString("Switch");
  QAction *switchAction = toolBarFichier->addAction(switchIcon, switchString);
  connect(switchAction, SIGNAL(triggered()), this, SLOT(CreateSwitch()));

  QIcon routerIcon(":/Ico/Router.png");
  QString routerString("Router");
  QAction *routerAction = toolBarFichier->addAction(routerIcon, routerString);
  connect(routerAction, SIGNAL(triggered()), this, SLOT(CreateRouter()));

  toolBarFichier->addSeparator();

  QIcon linkIcon(":/Ico/WiredLink.png");
  QString linkString("Wired Link");
  QAction *linkAction = toolBarFichier->addAction(linkIcon, linkString);
  connect(linkAction, SIGNAL(triggered()), this, SLOT(CreateWiredLink()));

  QIcon stasLinkIcon(":/Ico/Link.png");
  QString stasLinkString("Station Link");
  QAction *stasLinkAction = toolBarFichier->addAction(stasLinkIcon, stasLinkString);
  connect(stasLinkAction, SIGNAL(triggered()), this, SLOT(CreateWifiLink()));

  QIcon p2pLinkIcon(":/Ico/P2pLink.png");
  QString p2pLinkString("P2P Link");
  QAction *p2pLinkAction = toolBarFichier->addAction(p2pLinkIcon, p2pLinkString);
  connect(p2pLinkAction, SIGNAL(triggered()), this, SLOT(CreateP2pLink()));

  toolBarFichier->addSeparator();

  QIcon runLinkIcon("");
  QString runLinkString("Run");
  QAction *runLinkAction = toolBarFichier->addAction(runLinkIcon, runLinkString);
  connect(runLinkAction, SIGNAL(triggered()), this, SLOT(RunSimulation()));

  toolBarFichier->addSeparator();

  QIcon delIcon(":/Ico/Del.png");
  QString delString("Delete");
  this->m_delAction = toolBarFichier->addAction(delIcon, delString);
  this->m_delAction->setDisabled(true);
  connect(this->m_delAction, SIGNAL(triggered()), this, SLOT(DeleteObject()));

  //
  // 工具栏下方：4 个输入框 + 完成按钮
  //
  QHBoxLayout *inputLayout = new QHBoxLayout;

  this->m_topoFileEdit = new QLineEdit;
  this->m_topoFileEdit->setPlaceholderText("Topology File Path");
  inputLayout->addWidget(new QLabel("Topology File:"));
  inputLayout->addWidget(this->m_topoFileEdit);

  /*
  this->m_flowFileEdit = new QLineEdit;
  this->m_flowFileEdit->setPlaceholderText("Flow File Path");
  inputLayout->addWidget(new QLabel("Flow File:"));
  inputLayout->addWidget(this->m_flowFileEdit);
*/

  QPushButton *genFlowButton = new QPushButton("GenFlow");
  this->m_flowFileLabel = new QLabel("No flow file");
  connect(genFlowButton, &QPushButton::clicked, this, [=]()
          {
        QString trafficDir = QDir::currentPath() + "/../traffic_gen";
        FlowConfigDialog dialog(trafficDir, m_flowFilepath, m_flowFileLabel, m_statusLabel, this);
        dialog.exec(); });
  inputLayout->addWidget(new QLabel("Flow:"));
  inputLayout->addWidget(genFlowButton);
  inputLayout->addWidget(this->m_flowFileLabel);

  this->m_bandwidthEdit = new QLineEdit;
  this->m_bandwidthEdit->setPlaceholderText("Bandwidth (e.g., 10Mbps)");
  inputLayout->addWidget(new QLabel("Bandwidth:"));
  inputLayout->addWidget(this->m_bandwidthEdit);

  this->m_algorithmEdit = new QLineEdit;
  this->m_algorithmEdit->setPlaceholderText("Algorithm Name");
  inputLayout->addWidget(new QLabel("Algorithm:"));
  inputLayout->addWidget(this->m_algorithmEdit);

  QPushButton *submitButton = new QPushButton("Submit");
  connect(submitButton, SIGNAL(clicked()), this, SLOT(SubmitParameters()));
  inputLayout->addWidget(submitButton);

  inputLayout->addStretch(); // 靠左对齐，填充空白
  mainLayout->addLayout(inputLayout);

  //
  // 拖放区域
  //
  QHBoxLayout *dragLayout = new QHBoxLayout;
  this->m_dw = new DragWidget();
  dragLayout->addWidget(this->m_dw);
  mainLayout->addLayout(dragLayout);

  //
  // 设置中心部件
  //
  QWidget *zoneCentral = new QWidget;
  zoneCentral->setLayout(mainLayout);
  this->setCentralWidget(zoneCentral);

  //
  // 链接拖放部件
  //
  this->m_dw->SetMainWindow(this);
}

void MainWindow::SubmitParameters()
{
  QString topoFile = m_topoFileEdit->text();
  QFileInfo file{m_flowFilepath};
  QString flowFile = file.fileName();
  QString bandwidth = m_bandwidthEdit->text();
  QString algorithm = m_algorithmEdit->text();

  // do lots check

  m_topoFilepath = topoFile;
  m_flowFilepath = flowFile;
  m_bandwidth = bandwidth;
  m_algorithmName = algorithm;
  if (topoFile.isEmpty())
  {
    m_topoFilepath = "tmp_topology.txt";
    DefaultSaveTopology();
  }
  if (flowFile.isEmpty())
  {
    // m_flowFilepath = "tmp_traffic.txt";
    m_flowFilepath = "tmp_traffic.txt";
  }
  if (bandwidth.isEmpty())
  {
    m_bandwidth = "100";
  }
  if (algorithm.isEmpty())
  {
    m_algorithmName = "dcqcn";
  }

  m_topoFileEdit->setText(m_topoFilepath);
  // m_flowFileEdit->setText(m_flowFilepath);
  m_bandwidthEdit->setText(m_bandwidth);
  m_algorithmEdit->setText(m_algorithmName);
}

MainWindow::~MainWindow()
{
  delete this->m_gen;
}

void MainWindow::SetGenerator(Generator *gen)
{
  this->m_gen = gen;
}

Generator *MainWindow::GetGenerator()
{
  return this->m_gen;
}

void MainWindow::CleanIface()
{
  bool used = false;
  for (size_t i = 0; i < this->m_listIface.size(); i++)
  {
    used = false;
    for (size_t j = 0; j < this->m_gen->GetNNetworkHardwares(); j++)
    {
      if ((this->m_gen->GetNetworkHardware(j)->GetNetworkHardwareName()).find("tap_") == 0)
      {
        if (this->m_listIface.at(i) == static_cast<Tap *>(this->m_gen->GetNetworkHardware(j))->GetIfaceName())
        {
          used = true;
          break;
        }
      }
      if ((this->m_gen->GetNetworkHardware(j)->GetNetworkHardwareName()).find("emu_") == 0)
      {
        if (this->m_listIface.at(i) == static_cast<Emu *>(this->m_gen->GetNetworkHardware(j))->GetIfaceName())
        {
          used = true;
          break;
        }
      }
    }
    if (!used)
    {
      this->m_listIface.erase(this->m_listIface.begin() + i);
    }
  }
}

void MainWindow::CreatePc()
{
  this->m_gen->AddNode("Pc");
  this->m_dw->CreateObject("Pc", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName());
}

void MainWindow::CreatePcGroup()
{
  bool ok;
  size_t number = 0;
  QString text = QInputDialog::getText(this, "Terminal Group", tr("Enter the number of machines to create :"), QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty())
  {
    number = text.toUInt();
  }
  else
  {
    return;
  }

  if (number <= 0)
  {
    QMessageBox::about(this, "Error", "The pc number can't be negative ...");
    return;
  }

  this->m_gen->AddNode("Pc-group", number);
  this->m_dw->CreateObject("Pc-group", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName());
}

void MainWindow::CreateEmu()
{
  this->CleanIface();

  bool ok;
  QString text = QInputDialog::getText(this, "Emu", tr("Enter the real host interface to use:"), QLineEdit::Normal, "eth0", &ok);
  if (ok && !text.isEmpty())
  {
    for (size_t i = 0; i < this->m_listIface.size(); i++)
    {
      if (text.toStdString() == this->m_listIface.at(i))
      {
        QMessageBox::about(this, "Error", "The specified interface is already used ...");
        return;
      }
    }
    this->m_listIface.push_back(text.toStdString());
  }
  else
  {
    return;
  }

  this->m_gen->AddNode("Emu");
  this->m_gen->AddNetworkHardware("Emu", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName(), text.toStdString());
  this->m_dw->CreateObject("Emu", this->m_gen->GetNetworkHardware(this->m_gen->GetNNetworkHardwares() - 1)->GetNetworkHardwareName());
}

void MainWindow::CreateTap()
{
  this->CleanIface();

  bool ok;
  QString text = QInputDialog::getText(this, "Tap", tr("Enter the new interface to use :"), QLineEdit::Normal, "tap0", &ok);
  if (ok && !text.isEmpty())
  {
    for (size_t i = 0; i < this->m_listIface.size(); i++)
    {
      if (text.toStdString() == this->m_listIface.at(i))
      {
        QMessageBox::about(this, "Error", "The specified interface is already used ...");
        return;
      }
    }
    this->m_listIface.push_back(text.toStdString());
  }
  else
  {
    return;
  }

  this->m_gen->AddNode("Tap");
  this->m_gen->AddNetworkHardware("Tap", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName(), text.toStdString());
  this->m_dw->CreateObject("Tap", this->m_gen->GetNetworkHardware(this->m_gen->GetNNetworkHardwares() - 1)->GetNetworkHardwareName());
}

void MainWindow::CreateAp()
{
  this->m_gen->AddNode("Ap");
  this->m_gen->AddNetworkHardware("Ap", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName());
  this->m_dw->CreateObject("Ap", this->m_gen->GetNetworkHardware(this->m_gen->GetNNetworkHardwares() - 1)->GetNetworkHardwareName());
}

void MainWindow::CreateStation()
{
  this->m_gen->AddNode("Station");
  this->m_dw->CreateObject("Station", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName());
}

void MainWindow::CreateHub()
{
  this->m_gen->AddNetworkHardware("Hub");
  this->m_dw->CreateObject("Hub", this->m_gen->GetNetworkHardware(this->m_gen->GetNNetworkHardwares() - 1)->GetNetworkHardwareName());
}

void MainWindow::CreateSwitch()
{
  this->m_gen->AddNode("Bridge");
  this->m_gen->AddNetworkHardware("Bridge", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName());
  this->m_dw->CreateObject("Bridge", this->m_gen->GetNetworkHardware(this->m_gen->GetNNetworkHardwares() - 1)->GetNetworkHardwareName());
}

void MainWindow::CreateRouter()
{
  this->m_gen->AddNode("Router");
  this->m_dw->CreateObject("Router", this->m_gen->GetNode(this->m_gen->GetNNodes() - 1)->GetNodeName());
}

void MainWindow::CreateWiredLink()
{
  if (this->m_dw->GetTraceNetworkHardware())
  {
    this->m_dw->SetTraceNetworkHardware(false);
    this->m_dw->ResetSelected();
    return;
  }
  this->m_dw->SetTraceNetworkHardware(true);
  this->m_dw->SetNetworkHardwareType("WiredLink");
}

void MainWindow::CreateWifiLink()
{
  if (this->m_dw->GetTraceNetworkHardware())
  {
    this->m_dw->SetTraceNetworkHardware(false);
    this->m_dw->ResetSelected();
    return;
  }
  this->m_dw->SetTraceNetworkHardware(true);
  this->m_dw->SetNetworkHardwareType("WifiLink");
}

void MainWindow::CreateP2pLink()
{
  if (this->m_dw->GetTraceNetworkHardware())
  {
    this->m_dw->SetTraceNetworkHardware(false);
    this->m_dw->ResetSelected();
    return;
  }
  this->m_dw->SetTraceNetworkHardware(true);
  this->m_dw->SetNetworkHardwareType("P2pLink");
}

void MainWindow::CreateApplication()
{
  if (!this->m_dw->m_appsPing && !this->m_dw->m_appsUdpEcho && !this->m_dw->m_appsTcp)
  {
    this->m_appsDialog = new ApplicationDialog(this->m_dw);
    this->m_appsDialog->exec();
  }
}

void MainWindow::ConnectNode(const std::string &linkName, const std::string &nodeName)
{
  size_t linkNumber = -1;

  // search link
  for (size_t i = 0; i < this->m_gen->GetNNetworkHardwares(); i++)
  {
    if (linkName == this->m_gen->GetNetworkHardware(i)->GetNetworkHardwareName())
    {
      linkNumber = i;
      break;
    }
  }

  // get the params network connected equipements.
  size_t numberOfConnectedMachines = 0;
  if (nodeName.find("Get") == 0)
  {
    numberOfConnectedMachines += 1;
  }
  else
  {
    for (size_t i = 0; i < this->m_gen->GetNNodes(); i++)
    {
      if (nodeName == this->m_gen->GetNode(i)->GetNodeName())
      {
        numberOfConnectedMachines += this->m_gen->GetNode(i)->GetMachinesNumber();
      }
    }
  }

  // get the current (destination) network connected equipements.
  std::vector<std::string> nodes = this->m_gen->GetNetworkHardware(linkNumber)->GetInstalledNodes();
  for (size_t i = 0; i < nodes.size(); i++)
  {
    for (size_t j = 0; j < this->m_gen->GetNNodes(); j++)
    {
      if (nodes.at(i) == this->m_gen->GetNode(j)->GetNodeName())
      {
        numberOfConnectedMachines += this->m_gen->GetNode(j)->GetMachinesNumber();
      }
    }
  }

  // prevent overflow
  if (numberOfConnectedMachines > (255 - 2))
  {
    QMessageBox::about(this, "Error", "Limit of machines exceeded.");
    for (size_t i = 0; i < this->m_dw->GetDrawLines().size(); i++)
    {
      if ((nodeName == this->m_dw->GetDrawLine(i).GetFirst() &&
           this->m_gen->GetNetworkHardware(linkNumber)->GetNetworkHardwareName() == this->m_dw->GetDrawLine(i).GetSecond()) ||
          (this->m_gen->GetNetworkHardware(linkNumber)->GetNetworkHardwareName() == this->m_dw->GetDrawLine(i).GetFirst() &&
           nodeName == this->m_dw->GetDrawLine(i).GetSecond()))
      {
        this->m_dw->EraseDrawLine(i);
        return;
      }
    }
    return;
  }

  // install the params node to the dest network
  this->m_gen->GetNetworkHardware(linkNumber)->Install(nodeName);
}

void MainWindow::GenerateCpp()
{
  QString fileName = "";
  QFileDialog dlg(this, tr("Generate Cpp"));

  dlg.setFileMode(QFileDialog::AnyFile);
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix(".cc");

  if (dlg.exec())
  {
    fileName = dlg.selectedFiles().at(0);

    /* check if file exists and notificate the user */
    if (QFile(fileName).exists())
    {
      if (QMessageBox(QMessageBox::Question, "File exists", "File already exists. Overwrite ?",
                      QMessageBox::Ok | QMessageBox::No)
              .exec() != QMessageBox::Ok)
      {
        return;
      }
    }
  }

  this->m_gen->GenerateCodeCpp(fileName.toStdString());

  if (fileName != "")
  {
    QMessageBox(QMessageBox::Information, "Generated Cpp", "Code saved at " + fileName).exec();
  }
}

void MainWindow::GeneratePython()
{
  QString fileName = "";
  QFileDialog dlg(this, tr("Generate Python"));

  dlg.setFileMode(QFileDialog::AnyFile);
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix(".py");

  if (dlg.exec())
  {
    fileName = dlg.selectedFiles().at(0);

    /* check if file exists and notificate the user */
    if (QFile(fileName).exists())
    {
      if (QMessageBox(QMessageBox::Question, "File exists", "File already exists. Overwrite?",
                      QMessageBox::Ok | QMessageBox::No)
              .exec() != QMessageBox::Ok)
      {
        return;
      }
    }
  }

  this->m_gen->GenerateCodePython(fileName.toStdString());

  if (fileName != "")
  {
    QMessageBox(QMessageBox::Information, "Generated Python", "Code saved at " + fileName).exec();
  }
}

void MainWindow::DeleteObject()
{
  this->m_dw->DeleteSelected();
}

void MainWindow::About()
{
  QMessageBox::about(this, "About",
                     tr("<p align=\"center\">"
                        "<h2>The ns-3 topology generator"
                        "</h2>"
                        "</p><br />"
                        "Copyright (c) 2009-2010 University of Strasbourg<br /><br />"
                        "This program is free software; you can redistribute it and/or<br />"
                        "modify it under the terms of the GNU General Public License<br />"
                        "as published by the Free Software Foundation; either version 2<br />"
                        "of the License, or (at your option) any later version.<br />"
                        "<br />"
                        "<strong>Authors:</strong><br />"
                        "Pierre Weiss &lt;3weissp@gmail.com&gt;<br />"
                        "Sebastien Vincent &lt;vincent@clarinet.u-strasbg.fr&gt;"));
}

void MainWindow::SavePicture()
{
  QFileDialog dlg(this, tr("Save image (*.png)"));
  dlg.setFileMode(QFileDialog::AnyFile);
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix(".png");

  if (dlg.exec())
  {
    QImage img = this->m_dw->grab().toImage();
    QString fileName = dlg.selectedFiles().at(0);

    /* check if file exists and notificate the user */
    if (QFile(fileName).exists())
    {
      if (QMessageBox(QMessageBox::Question, "File exists", "File already exists. Overwrite?",
                      QMessageBox::Ok | QMessageBox::No)
              .exec() != QMessageBox::Ok)
      {
        return;
      }
    }

    if (img.save(fileName))
    {
      QMessageBox(QMessageBox::Information, "Save picture", "Picture saved at " + fileName).exec();
    }
    else
    {
      QMessageBox(QMessageBox::Warning, "Save picture", "Picture saving failed!").exec();
    }
  }
}

void MainWindow::SaveTxt()
{
  QString fileName = "";
  QFileDialog dlg(this, tr("Save TXT"));
  dlg.setFileMode(QFileDialog::AnyFile);
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix(".txt");

  if (dlg.exec())
  {
    fileName = dlg.selectedFiles().at(0);

    /* check if file exists and notificate the user */
    if (QFile(fileName).exists())
    {
      if (QMessageBox(QMessageBox::Question, "File exists", "File already exists. Overwrite?",
                      QMessageBox::Ok | QMessageBox::No)
              .exec() != QMessageBox::Ok)
      {
        return;
      }
    }
  }

  // QString fileName = "test.txt";
  QFile file(fileName);
  file.open(QFile::WriteOnly | QFile::Text);
  // QXmlStreamWriter *writer = new QXmlStreamWriter(&file);

  guiUtils::saveTxt(file.fileName(), this->m_gen, this->m_dw);

  file.close();

  QMessageBox(QMessageBox::Information, "Save Simulation", "Simulation saved at " + fileName).exec();
}

void MainWindow::SaveXml()
{
  QString fileName = "";
  QFileDialog dlg(this, tr("Save XML"));
  dlg.setFileMode(QFileDialog::AnyFile);
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix(".xml");

  if (dlg.exec())
  {
    fileName = dlg.selectedFiles().at(0);

    /* check if file exists and notificate the user */
    if (QFile(fileName).exists())
    {
      if (QMessageBox(QMessageBox::Question, "File exists", "File already exists. Overwrite?",
                      QMessageBox::Ok | QMessageBox::No)
              .exec() != QMessageBox::Ok)
      {
        return;
      }
    }
  }

  // QString fileName = "test.xml";
  QFile file(fileName);
  file.open(QFile::WriteOnly | QFile::Text);
  QXmlStreamWriter *writer = new QXmlStreamWriter(&file);

  guiUtils::saveXml(writer, this->m_gen, this->m_dw);

  file.close();

  QMessageBox(QMessageBox::Information, "Save Simulation", "Simulation saved at " + fileName).exec();
}

void MainWindow::LoadXml()
{
  QString fileName = "";
  QFileDialog dlg(this, tr("Load XML"));
  dlg.setFileMode(QFileDialog::AnyFile);

  if (dlg.exec())
  {
    fileName = dlg.selectedFiles().at(0);

    if (!QFile(fileName).exists())
    {
      QMessageBox(QMessageBox::Information, "File don't exists", "File don't exists.").exec();
      return;
    }
  }

  QFile file(fileName);
  file.open(QFile::ReadOnly | QFile::Text);
  QXmlStreamReader *reader = new QXmlStreamReader(&file);

  guiUtils::loadXml(reader, this->m_gen, this->m_dw);

  file.close();

  QMessageBox(QMessageBox::Information, "Load Simulation", "Simulation loaded.").exec();
}

void MainWindow::SelectTopoFile()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("选择拓扑文件"), ".", tr("拓扑文件 (*.txt)"));
  if (!fileName.isEmpty())
  {
    m_topoFilepath = fileName;
    qDebug() << "Selected Topo File:" << m_topoFilepath;
  }
}

void MainWindow::SelectFlowFile()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("选择流文件"), ".", tr("流文件 (*.txt)"));
  if (!fileName.isEmpty())
  {
    m_flowFilepath = fileName;
    qDebug() << "Selected Flow File:" << m_flowFilepath;
  }
}

// void MainWindow::RunSimulation(){}

void MainWindow::RunSimulation()
{
  qDebug() << "Run 按钮被点击！";

  // 示例命令：执行 python run.py --cc dcqcn --topo topoFile --trace flowFile
  QStringList arguments;
  QFileInfo topo(m_topoFilepath);
  QFileInfo flow(m_flowFilepath);
  arguments << "run.py"
            << "--cc" << m_algorithmName
            << "--topo" << topo.completeBaseName()
            << "--trace" << flow.completeBaseName()
            << "--bw" << m_bandwidth;

  QProcess *process = new QProcess(this);
  process->setWorkingDirectory(QDir::currentPath());

  qDebug() << "执行命令: python" << arguments.join(" ");

  // process->start("python", arguments);

  // 创建进度对话框 int x{new int{}};
  // QProgressDialog *progressDialog = new QProgressDialog{"Running simulation...", "Cancel", 0, 0, this};
  // progressDialog->setWindowTitle("Simulation in Progress");
  // progressDialog->setWindowModality(Qt::WindowModal); // 模态对话框，阻止用户操作主窗口
  // progressDialog->setAutoClose(false);                // 手动控制关闭
  // progressDialog->setAutoReset(false);
  progressDialog->setLabelText("simulating...");
  progressDialog->show(); // 显示进度对话框

  // 状态标志：区分用户取消和程序关闭
  bool userCanceled = false;

  // 连接取消按钮
  connect(progressDialog, &QProgressDialog::canceled, this, [=]() mutable
          {
            userCanceled = true;
            process->kill();
            qDebug() << "Process canceled by user";
            progressDialog->hide(); // 隐藏对话框
          });

  // 连接 QProcess 信号
  connect(process, &QProcess::started, this, [=]()
          {
            qDebug() << "执行命令: python" << arguments.join(" ");
            progressDialog->setLabelText("Python 脚本已启动..."); });

  connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, [=](int exitCode, QProcess::ExitStatus exitStatus)
          {
                    qDebug() << "Finished signal, exitCode:" << exitCode << ", exitStatus:" << exitStatus;

                    // 隐藏对话框，防止触发 canceled 信号
                    progressDialog->hide();

                    QString message;
                    if (exitStatus == QProcess::NormalExit && exitCode == 0 && !userCanceled) {
                        message = "仿真成功完成！";
                    }
                    // 显示消息框
                    QMessageBox::information(this, "仿真结果", message);
                    // 清理
                    process->deleteLater(); });

  connect(process, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error)
          {
            qDebug() << "ErrorOccurred signal, error:" << process->errorString();

            // 隐藏对话框
            progressDialog->hide();

            QMessageBox::critical(this, "错误", QString("无法启动仿真: %1")
                                                  .arg(process->errorString()));

            // 清理
            process->deleteLater(); });

  // 启动进程
  process->start("python", arguments);

  if (!process->waitForStarted())
  {
    qDebug() << "启动 Python 脚本失败！";
    progressDialog->hide();
    QMessageBox::critical(this, "错误", "无法启动 Python 脚本！");
    process->deleteLater();
    return;
  }
}

void MainWindow::DefaultSaveTopology()
{
  // 创建临时文件 tmp_topology.txt
  QString tempFilePath = QDir::currentPath() + "/mix/tmp_topology.txt";
  QFile file(tempFilePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QMessageBox::warning(this, "Error", "Failed to create temporary topology file!");
    return;
  }
  file.open(QFile::WriteOnly | QFile::Text);
  // QXmlStreamWriter *writer = new QXmlStreamWriter(&file);

  guiUtils::saveTxt(file.fileName(), this->m_gen, this->m_dw);

  file.close();
}

void MainWindow::onFctSlowDownTriggered()
{
  FctSlowDownDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted)
  {
    QStringList ccs = dialog.getFilePaths();
    QStringList algNames = dialog.getAlgNames();

    if (ccs.isEmpty())
    {
      QMessageBox::warning(this, "错误", "请至少选择一个文件！");
      return;
    }
    if (ccs.size() != algNames.size())
    {
      QMessageBox::warning(this, "错误", "文件数量与算法名称数量不匹配！");
      return;
    }
    for (const QString &algName : algNames)
    {
      if (algName.isEmpty())
      {
        QMessageBox::warning(this, "错误", "所有算法名称不能为空！");
        return;
      }
    }

    runPythonFctPipeline("5", 0, 3000000000, 25, ccs, algNames, "./charts");
  }
}

void MainWindow::runPythonFctPipeline(const QString &step, int type, int timeLimit, int bandwidth, const QStringList &ccs, const QStringList &algNames, const QString &outputDir)
{
  QProcess process;
  QStringList arguments;

  //  arguments << "-s" << step
  //            << "-t" << QString::number(type)
  //            << "-T" << QString::number(timeLimit)
  //            << "-b" << QString::number(bandwidth)
  //            << "--ccs" << ccs.join(",")
  //            << "--alg-names" << algNames.join(",")
  //            << "--output-dir" << outputDir;
  //
  arguments << "--ccs" << ccs.join(",")
            << "--alg-names" << algNames.join(",")
            << "--output-dir" << "./charts/";
  qDebug() << "运行 Python FCT Slowdown Pipeline，参数：" << arguments.join(" ");
  process.start("python3", QStringList() << "fct_slowdown_pipeline.py" << arguments); // 替换为实际 Python 脚本路径
  process.waitForFinished(-1);

  if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
  {
    qDebug() << "Python 脚本执行成功。";
    qDebug() << "输出:" << process.readAllStandardOutput();

    // 显示生成的图片
    QStringList imagePaths = {
        outputDir + "/avg_fct.png",
        outputDir + "/95_fct.png",
        outputDir + "/99_fct.png"};
    ImageViewer viewer(imagePaths, this);
    viewer.exec();
  }
  else
  {
    QMessageBox::critical(this, "错误", "执行 Python 脚本失败:\n" + process.errorString() + "\n" + process.readAllStandardError());
  }
}


void MainWindow::ThoughtPut() {
	 Thoughtput *dialog = new Thoughtput(this);
	 dialog->exec(); // 模态对话框方式打开	
}
