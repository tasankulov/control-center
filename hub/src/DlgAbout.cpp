#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include "Commons.h"
#include "DlgAbout.h"
#include "ui_DlgAbout.h"
#include "SystemCallWrapper.h"
#include "SettingsManager.h"
#include "updater/HubComponentsUpdater.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"

using namespace update_system;

QString get_p2p_version() {
  QString p2p_version = "";
  CSystemCallWrapper::p2p_version(p2p_version);

  p2p_version = p2p_version.remove("p2p");
  p2p_version = p2p_version.remove("version");
  p2p_version = p2p_version.remove("  ");

  return p2p_version;
}

QString get_x2go_version(){
    QString x2go_version = "";
    CSystemCallWrapper::x2go_version(x2go_version);
    return x2go_version;
}

QString get_vagrant_version(){
    QString vagrant_version = "";
    CSystemCallWrapper::vagrant_version(vagrant_version);
    return vagrant_version;
}

QString get_oracle_virtualbox_version(){
    QString version = "";
    CSystemCallWrapper::oracle_virtualbox_version(version);
    return version;
}
////////////////////////////////////////////////////////////////////////////

DlgAbout::DlgAbout(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgAbout)
{
  ui->setupUi(this);
  ui->lbl_tray_version_val->setText(TRAY_VERSION + branch_name_str());

  this->setMinimumWidth(700);

  ui->gridLayout->setSizeConstraint(QLayout::SetMinimumSize);
  ui->gridLayout_2->setSizeConstraint(QLayout::SetMinimumSize);
  ui->gridLayout_3->setSizeConstraint(QLayout::SetMinimumSize);

  QLabel* lbls[]= { this->ui->lbl_chrome_version_val,
                    this->ui->lbl_p2p_version_val,
                    this->ui->lbl_rhm_version_val,
                    this->ui->lbl_rh_version_val,
                    this->ui->lbl_tray_version_val,
                    this->ui->lbl_x2go_version_val,
                    this->ui->lbl_vagrant_version_val,
                    nullptr };

  for (QLabel **i = lbls; *i; ++i) {
    (*i)->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    (*i)->setWordWrap(true);
  }
  bool p2p_visible = CSettingsManager::Instance().p2p_path() != snap_p2p_path();
  ui->btn_p2p_update->setVisible(p2p_visible);
  ui->pb_p2p->setVisible(p2p_visible);

  //connect
  connect(ui->btn_p2p_update, &QPushButton::released, this, &DlgAbout::btn_p2p_update_released);
  connect(ui->btn_tray_update, &QPushButton::released, this, &DlgAbout::btn_tray_update_released);
  connect(ui->btn_rh_update, &QPushButton::released, this, &DlgAbout::btn_rh_update_released);
  connect(ui->btn_rhm_update, &QPushButton::released, this, &DlgAbout::btn_rhm_update_released);
  connect(ui->btn_recheck, &QPushButton::released, this, &DlgAbout::btn_recheck_released);
  connect(ui->btn_x2go_update, &QPushButton::released, this, &DlgAbout::btn_x2go_update_released);
  connect(ui->btn_vagrant_update, &QPushButton::released, this, &DlgAbout::btn_vagrant_update_released);
  connect(ui->btn_oracle_virtualbox_update, &QPushButton::released, this, &DlgAbout::btn_oracle_virtualbox_update_released);

  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::download_file_progress,
          this, &DlgAbout::download_progress);
  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::update_available,
          this, &DlgAbout::update_available);
  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::updating_finished,
          this, &DlgAbout::update_finished);
  connect(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::installing_finished,
          this, &DlgAbout::install_finished);

  static bool p2p_in_progress = false;
  static bool tray_in_progress = false;
  static bool rh_in_progress = false;
  static bool rhm_in_progress = false;
  static bool x2go_in_progress = false;
  static bool vagrant_in_progress = false;
  static bool oracle_virtualbox_in_progress = false;

  m_dct_fpb[IUpdaterComponent::P2P] = {ui->lbl_p2p_version_val, ui->pb_p2p, ui->btn_p2p_update,
                                       &p2p_in_progress, get_p2p_version};
  m_dct_fpb[IUpdaterComponent::TRAY] = {ui->lbl_tray_version_val, ui->pb_tray, ui->btn_tray_update,
                                        &tray_in_progress, NULL};
  m_dct_fpb[IUpdaterComponent::RH] = {ui->lbl_rh_version_val, ui->pb_rh, ui->btn_rh_update,
                                      &rh_in_progress, CSystemCallWrapper::rh_version};
  m_dct_fpb[IUpdaterComponent::RHMANAGEMENT] = {ui->lbl_rhm_version_val, ui->pb_rhm, ui->btn_rhm_update,
                                                &rhm_in_progress, CSystemCallWrapper::rhm_version};
  m_dct_fpb[IUpdaterComponent::X2GO] = {ui->lbl_x2go_version_val, ui->pb_x2go, ui->btn_x2go_update,
                                       &x2go_in_progress, get_x2go_version};
  m_dct_fpb[IUpdaterComponent::VAGRANT] = {ui->lbl_vagrant_version_val, ui->pb_vagrant, ui->btn_vagrant_update,
                                          &vagrant_in_progress, get_vagrant_version};
  m_dct_fpb[IUpdaterComponent::ORACLE_VIRTUALBOX] = {ui->lbl_oracle_virtualbox_version_val, ui->pb_oracle_virtualbox, ui->btn_oracle_virtualbox_update,
                                          &oracle_virtualbox_in_progress, get_oracle_virtualbox_version};

  ui->pb_initialization_progress->setMaximum(DlgAboutInitializer::COMPONENTS_COUNT);
  check_for_versions_and_updates();
}

DlgAbout::~DlgAbout() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::check_for_versions_and_updates() {
  ui->btn_recheck->setEnabled(false);
  ui->pb_initialization_progress->setEnabled(true);
  DlgAboutInitializer* di = new DlgAboutInitializer();

  connect(di, &DlgAboutInitializer::finished,
          this, &DlgAbout::initialization_finished);
  connect(di, &DlgAboutInitializer::got_chrome_version,
          this, &DlgAbout::got_chrome_version_sl);
  connect(di, &DlgAboutInitializer::got_p2p_version,
          this, &DlgAbout::got_p2p_version_sl);
  connect(di, &DlgAboutInitializer::got_rh_version,
          this, &DlgAbout::got_rh_version_sl);
  connect(di, &DlgAboutInitializer::got_rh_management_version,
          this, &DlgAbout::got_rh_management_version_sl);
  connect(di, &DlgAboutInitializer::got_x2go_version,
          this, &DlgAbout::got_x2go_version_sl);
  connect(di, &DlgAboutInitializer::got_vagrant_version,
          this, &DlgAbout::got_vagrant_version_sl);
  connect(di, &DlgAboutInitializer::got_oracle_virtualbox_version,
          this, &DlgAbout::got_oracle_virtualbox_version_sl);
  connect(di, &DlgAboutInitializer::update_available,
          this, &DlgAbout::update_available_sl);
  connect(di, &DlgAboutInitializer::init_progress,
          this, &DlgAbout::init_progress_sl);
  di->startWork();
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_tray_update_released() {
  ui->btn_tray_update->setEnabled(false);
  *m_dct_fpb[IUpdaterComponent::TRAY].in_progress = true;
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::TRAY);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_p2p_update_released() {
  ui->btn_p2p_update->setEnabled(false);
  *m_dct_fpb[IUpdaterComponent::P2P].in_progress = true;
  if(ui->lbl_p2p_version_val->text()=="undefined")
      CHubComponentsUpdater::Instance()->install(IUpdaterComponent::P2P);
  else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::P2P);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_rh_update_released() {
  ui->btn_rh_update->setEnabled(false);
  ui->pb_rh->setEnabled(false);
  ui->pb_rh->setRange(0, 0);
  *m_dct_fpb[IUpdaterComponent::RH].in_progress = true;
  QtConcurrent::run(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::force_update,
                    IUpdaterComponent::RH);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_rhm_update_released() {
  ui->btn_rhm_update->setEnabled(false);
  ui->pb_rhm->setEnabled(false);
  ui->pb_rhm->setRange(0,0);
  *m_dct_fpb[IUpdaterComponent::RHMANAGEMENT].in_progress = true;
  QtConcurrent::run(CHubComponentsUpdater::Instance(), &CHubComponentsUpdater::force_update,
                    IUpdaterComponent::RHMANAGEMENT);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::btn_x2go_update_released() {
    ui->btn_x2go_update->setEnabled(false);
    *m_dct_fpb[IUpdaterComponent::X2GO].in_progress = true;
    if(ui->lbl_x2go_version_val->text()=="undefined")
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::X2GO);
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::X2GO);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::btn_vagrant_update_released(){
    ui->btn_vagrant_update->setEnabled(false);
    *m_dct_fpb[IUpdaterComponent::VAGRANT].in_progress = true;
    if(ui->lbl_vagrant_version_val->text()=="undefined")
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::VAGRANT);
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::VAGRANT);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::btn_oracle_virtualbox_update_released(){
    ui->btn_oracle_virtualbox_update->setEnabled(false);
    *m_dct_fpb[IUpdaterComponent::ORACLE_VIRTUALBOX].in_progress = true;
    if(ui->lbl_oracle_virtualbox_version_val->text()=="undefined")
        CHubComponentsUpdater::Instance()->install(IUpdaterComponent::ORACLE_VIRTUALBOX);
    else CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::ORACLE_VIRTUALBOX);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::btn_recheck_released() {
  check_for_versions_and_updates();
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::download_progress(const QString& file_id,
                            qint64 rec,
                            qint64 total) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].pb->setValue((rec*100)/total);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_available(const QString& file_id) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].pb->setEnabled(!(*m_dct_fpb[file_id].in_progress));
  m_dct_fpb[file_id].pb->setEnabled(!(*m_dct_fpb[file_id].in_progress));
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_finished(const QString& file_id,
                          bool success) {
  if (!success) {
    QString template_str = tr("Couldn't update component %1");
    CNotificationObserver::Error(template_str.arg(file_id), DlgNotification::N_NO_ACTION);
  }

  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].btn->setEnabled(false);
  m_dct_fpb[file_id].pb->setEnabled(false);
  m_dct_fpb[file_id].pb->setValue(0);
  m_dct_fpb[file_id].pb->setRange(0, 100);
  *m_dct_fpb[file_id].in_progress = false;
  if (m_dct_fpb[file_id].pf_version) {
    m_dct_fpb[file_id].lbl->setText(m_dct_fpb[file_id].pf_version());
  }
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::initialization_finished() {
  ui->lbl_about_init->setEnabled(false);
  ui->pb_initialization_progress->setEnabled(false);
  ui->btn_recheck->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::init_progress_sl(int part,
                           int total) {
  UNUSED_ARG(total);
  ui->pb_initialization_progress->setValue(part);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_p2p_version_sl(QString version) {
  ui->lbl_p2p_version_val->setText(version);
  if(version == "undefined"){
      ui->btn_p2p_update->setText(tr("Install P2P"));
  }
  else ui->btn_p2p_update->setText(tr("Update P2P"));
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_chrome_version_sl(QString version) {
  ui->lbl_chrome_version_val->setText(version);
}

////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_rh_version_sl(QString version) {
  ui->lbl_rh_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_rh_management_version_sl(QString version) {
  ui->lbl_rhm_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void DlgAbout::got_x2go_version_sl(QString version){
    if(version == "undefined")
        ui->btn_x2go_update->setText(tr("Install X2Go-Client"));
    else ui->btn_x2go_update->setText(tr("Update X2Go-Client"));
    ui->lbl_x2go_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::got_vagrant_version_sl(QString version){
    if(version == "undefined")
        ui->btn_vagrant_update->setText(tr("Install Vagrant"));
    else ui->btn_vagrant_update->setText(tr("Update Vagrant"));
    ui->lbl_vagrant_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::got_oracle_virtualbox_version_sl(QString version){
    if(version == "undefined")
        ui->btn_oracle_virtualbox_update->setText(tr("Install Oracle VirtualBox"));
    else ui->btn_oracle_virtualbox_update->setText(tr("Update Oracle VirtualBox"));
    ui->lbl_oracle_virtualbox_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::update_available_sl(const QString& component_id,
                              bool available) {
  auto item = m_dct_fpb.find(component_id);
  if (item == m_dct_fpb.end()) return;
  item->second.pb->setEnabled(!(*item->second.in_progress) && available);
  item->second.btn->setEnabled(!(*item->second.in_progress) && available);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void
DlgAboutInitializer::do_initialization() {
  try {
    int initialized_component_count = 0;
    QString p2p_version = get_p2p_version();
    emit got_p2p_version(p2p_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString chrome_version;
    CSystemCallWrapper::chrome_version(chrome_version);

    emit got_chrome_version(chrome_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);


    QString rh_version = CSystemCallWrapper::rh_version();
    emit got_rh_version(rh_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString rhm_version = CSystemCallWrapper::rhm_version();
    emit got_rh_management_version(rhm_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString x2go_version = get_x2go_version();
    emit got_x2go_version(x2go_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString vagrant_version = get_vagrant_version();
    emit got_vagrant_version(vagrant_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString oracle_virtualbox_version = get_oracle_virtualbox_version();
    emit got_oracle_virtualbox_version(oracle_virtualbox_version);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

    QString uas[] = {
      IUpdaterComponent::P2P, IUpdaterComponent::TRAY,
      IUpdaterComponent::RH, IUpdaterComponent::RHMANAGEMENT, IUpdaterComponent::X2GO, IUpdaterComponent::VAGRANT, IUpdaterComponent::ORACLE_VIRTUALBOX, ""};

    for (int i = 0; uas[i] != ""; ++i) {
      bool ua = CHubComponentsUpdater::Instance()->is_update_available(uas[i]);
      emit update_available(uas[i], ua);
      emit init_progress(++initialized_component_count, COMPONENTS_COUNT);
    }
  } catch (std::exception& ex) {
    qCritical("Err in DlgAboutInitializer::do_initialization() . %s", ex.what());
  }

  emit finished();
}
////////////////////////////////////////////////////////////////////////////
void DlgAbout::install_finished(const QString &file_id, bool success){
    qDebug()<<"Install finished  for"<<file_id<<"with result"<<success;
    if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
    m_dct_fpb[file_id].btn->setEnabled(false);
    m_dct_fpb[file_id].pb->setEnabled(false);
    m_dct_fpb[file_id].pb->setValue(0);
    m_dct_fpb[file_id].pb->setRange(0, 100);
    *m_dct_fpb[file_id].in_progress = false;
    m_dct_fpb[file_id].btn->setText(tr("Update %1").arg(CHubComponentsUpdater::Instance()->component_name(file_id)));
    if (m_dct_fpb[file_id].pf_version) {
      m_dct_fpb[file_id].lbl->setText(m_dct_fpb[file_id].pf_version());
    }

}
////////////////////////////////////////////////////////////////////////////
void
DlgAboutInitializer::abort() {
  emit finished();
}
