#ifndef HUBCOMPONENTSUPDATER_H
#define HUBCOMPONENTSUPDATER_H

#include <QObject>
#include <QTimer>
#include <map>

#include "updater/IUpdaterComponent.h"
#include "SettingsManager.h"

namespace update_system {

  /*!
   * \brief The CUpdaterComponentItem class wraps IUpdaterComponent*.
   * Objects of this class are used in CHubComponentsUpdater in dictionary.
   */
  class CUpdaterComponentItem : public QObject {
    Q_OBJECT
  private:
    QTimer m_timer;
    IUpdaterComponent* m_component;

  public:
    //PUBLIC FIELD!!! ACHTUNG!!! :-D
    bool autoupdate;

    CUpdaterComponentItem() : m_component(NULL), autoupdate(false){
      connect(&m_timer, SIGNAL(timeout()), this, SLOT(timer_timeout_sl()));
    }

    explicit CUpdaterComponentItem(IUpdaterComponent* component) :
      m_component(component), autoupdate(false) {
      connect(&m_timer, SIGNAL(timeout()), this, SLOT(timer_timeout_sl()));
    }

    CUpdaterComponentItem(const CUpdaterComponentItem& arg); //copy constructor prohibited

    ~CUpdaterComponentItem(){}

    CUpdaterComponentItem& operator=(const CUpdaterComponentItem& rh) {
      this->m_component = rh.m_component;
      return *this;
    }

    void set_timer_interval(int msec) {
      m_timer.setInterval(msec);
    }

    void timer_start() {m_timer.start();}
    void timer_stop() {m_timer.stop();}

    IUpdaterComponent* Component() const {return m_component;}

  private slots:
    void timer_timeout_sl() {
      QString cid = m_component ? m_component->component_id() : "component_eq_null";
      emit timer_timeout(cid);
    }
  signals:
    void timer_timeout(const QString& component_id);
  };
  ////////////////////////////////////////////////////////////////////////////

  /*!
   * \brief This class provides functions and methods for managing component updating
   */
  class CHubComponentsUpdater : public QObject {
    Q_OBJECT
  private:

    CHubComponentsUpdater();
    ~CHubComponentsUpdater();

    std::map<QString, CUpdaterComponentItem> m_dct_components;
    void set_update_freq(const QString& component_id, CSettingsManager::update_freq_t freq);
    void set_component_autoupdate(const QString& component_id,
                                  bool autoupdate);

  public:

    /*!
     * \brief Instance of this singleton class
     */
    static CHubComponentsUpdater* Instance() {
      static CHubComponentsUpdater instance;
      return &instance;
    }

    /*!
     * \brief Set p2p update frequency using SettingsManager
     */
    void set_p2p_update_freq();

    /*!
     * \brief Set resource host update frequency using SettingsManager
     */
    void set_rh_update_freq();

    /*!
     * \brief Set tray application update frequency using SettingsManager
     */
    void set_tray_update_freq();

    void set_rh_management_update_freq();

    /*!
     * \brief Set p2p autoupdate using SettingsManager
     */
    void set_p2p_autoupdate();

    /*!
     * \brief Set resource host autoupdate using SettingsManager
     */
    void set_rh_autoupdate();

    /*!
     * \brief Set tray application autoupdate using SettingsManager
     */
    void set_tray_autoupdate();

    void set_rh_management_autoupdate();
    /*!
     * \brief Is update available for component with component_id identificator
     */
    bool is_update_available(const QString& component_id);

    /*!
     * \brief Run update procedure for component with component_id identificator
     */
    void force_update(const QString& component_id);

  private slots:

    void update_component_timer_timeout(const QString& component_id);
    void update_component_progress_sl(const QString &file_id, qint64 cur, qint64 full);
    void update_component_finished_sl(const QString &file_id, bool replaced);

  signals:
    void download_file_progress(const QString& file_id, qint64 rec, qint64 total);
    void updating_finished(const QString& file_id, bool success);
    void update_available(const QString& file_id);
  };
}

#endif // HUBCOMPONENTSUPDATER_H
