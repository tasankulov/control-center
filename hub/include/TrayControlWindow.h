#ifndef TRAYCONTROLWINDOW_H
#define TRAYCONTROLWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <VBox/com/string.h>
#include <QObject>
#include <QTimer>
#include <QMenu>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "IVirtualMachine.h"
#include "HubStatisticWindow.h"
#include "RestWorker.h"


namespace Ui {
  class TrayControlWindow;
}


class CVBPlayerItem : public QWidget {
    Q_OBJECT
private:
    com::Bstr m_vm_player_item_id;
    QAction* m_player_item_act;
//    QLabel *pLabelName;
//    QLabel *pLabelState;
//    QPushButton *pPlay;
//    QPushButton *pStop;

public:
    CVBPlayerItem(const IVirtualMachine* vm, QWidget* parent);
    virtual ~CVBPlayerItem();
    void set_buttons(ushort state);
    QAction* action() {return m_player_item_act;}
    QHBoxLayout *p_h_Layout;
    QLabel *pLabelName;
    QLabel *pLabelState;
    QPushButton *pPlay;
    QPushButton *pStop;

signals:
    void vbox_menu_btn_play_released_signal(const com::Bstr& vm_id);
    void vbox_menu_btn_stop_released_signal(const com::Bstr& vm_id);
public slots:
    void vbox_menu_btn_play_released();
    void vbox_menu_btn_stop_released();
//    void vbox_menu_btn_play(const com::Bstr& vm_id);
//    void vbox_menu_btn_stop(const com::Bstr& vm_id);
};

class CVBPlayer : public QWidget{
    Q_OBJECT
private:
    com::Bstr m_vm_player_id;
    QVBoxLayout *p_v_Layout;
    QAction* m_player_act;

public:
    CVBPlayer(QWidget *parent);
    virtual ~CVBPlayer();
    void add(CVBPlayerItem* pItem);
    void remove(CVBPlayerItem* pItem);
};


class CVboxMenu : public QMenu {
  Q_OBJECT
private:
  com::Bstr m_id;
  QAction* m_act;
//  QAction* m_player_act;

public:
  CVboxMenu(const IVirtualMachine *vm, QWidget *parent);
  virtual ~CVboxMenu();
  void set_machine_stopped(bool stopped);
  QAction* action() {return m_act;}
  void subMenu(const IVirtualMachine *vm, QMenu* parent);

signals:
  void vbox_menu_act_triggered(const com::Bstr& vm_id);
private slots:
  void act_triggered();

};

////////////////////////////////////////////////////////////////////////
//class VBoxPlayerMenu : public QMenu {
//  Q_OBJECT
//private:
//  com::Bstr m_player_id;
//  QAction* m_player_act;

//public:
//  VBoxPlayerMenu(const IVirtualMachine *vm, QWidget *parent);
//  virtual ~VBoxPlayerMenu();
//  void set_machine_state(int state);
//  QAction* action() {return m_player_act;}

//signals:
//  void vbox_play_act_triggered(const com::Bstr& vm_id);
//private slots:
//  void act_play_triggered();
//};

////////////////////////////////////////////////////////////////////////////

class CHubEnvironmentMenuItem : public QObject {
 Q_OBJECT
private:
  const CSSEnvironment* m_hub_environment;
  const CHubContainer* m_hub_container;

public:
  explicit CHubEnvironmentMenuItem(const CSSEnvironment* env,
                                   const CHubContainer* cont) :
    m_hub_environment(env), m_hub_container(cont){}

  ~CHubEnvironmentMenuItem(){}

signals:
  void action_triggered(const CSSEnvironment*, const CHubContainer*);

public slots:
  void internal_action_triggered();

};
////////////////////////////////////////////////////////////////////////////

class TrayControlWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TrayControlWindow(QWidget *parent = 0);
  ~TrayControlWindow();

private:
  Ui::TrayControlWindow *ui;
  QVBoxLayout *m_w_Layout;
  CVBPlayer *m_w_Player;
  /*hub*/
//  HubStatisticWindow m_hub_window;
  QTimer m_refresh_timer;
  std::vector<CSSEnvironment> m_lst_environments;
  std::vector<CHubEnvironmentMenuItem*> m_lst_hub_menu_items;
  /*hub end*/

  /*vbox*/
  std::map<com::Bstr, CVboxMenu*> m_dct_vm_menus;
//  std::map<com::Bstr, VBoxPlayerMenu*> m_player_menus;
  std::map<com::Bstr, CVBPlayerItem*>  m_dct_player_menus;
  void add_vm_menu(const com::Bstr &vm_id);
  void remove_vm_menu(const com::Bstr &vm_id);

  /*vbox end*/

  /*tray icon*/
  QMenu *m_hub_menu;
  QMenu *m_vbox_menu;
  QMenu *m_player_menu;
  QAction *m_hub_section;
  QAction *m_vbox_section;
  QAction *m_quit_section;

  QAction *m_act_quit;
  QAction *m_act_settings;
  QAction *m_act_vbox;
  QAction *m_act_hub;

  QSystemTrayIcon* m_sys_tray_icon;
  QMenu* m_tray_menu;  

  void create_tray_actions();
  void create_tray_icon();
  void fill_vm_menu();

  int IconPlace[4], TrayPlace[4], VboxPlace[4];
  /*tray icon end*/

private slots:
  /*tray slots*/
  void show_settings_dialog();
  void show_hub();
//  void join_to_swarm();
  void show_vbox();

  /*virtualbox slots*/
  void vm_added(const com::Bstr& vm_id);
  void vm_removed(const com::Bstr& vm_id);
  void vm_state_changed(const com::Bstr& vm_id);
  void vm_session_state_changed(const com::Bstr& vm_id);
  void vmc_act_released(const com::Bstr& vm_id);
  void vmc_player_act_released(const com::Bstr& vm_id);
  void vbox_menu_btn_play_triggered(const com::Bstr& vm_id);
  void vbox_menu_btn_stop_triggered(const com::Bstr& vm_id);

  /*hub slots*/
  void refresh_timer_timeout();
  void hub_menu_item_triggered(const CSSEnvironment *env,
                               const CHubContainer *cont);
};

#endif // TRAYCONTROLWINDOW_H


