#include <libudev.h>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <assert.h>

#include <thread>
#include <unistd.h>
#include <string.h>

#include <atomic>
#include <errno.h>
#include <cerrno>

#include <sys/mount.h>
#include <filesystem>

//Класс отвечающий за автоматическое монтирование в режиме демона
class Mount_Daemon{
  public:
    Mount_Daemon();
    ~Mount_Daemon();

    //Если мы были вызваны как root, то используется second_logname s
    void init  (const std::string& second_logname, bool is_daemon_mode = false);
    void reinit(const std::string& second_logname, bool is_daemon_mode = false);

    //стандартные методы запуска/остановки сервиса автоматического монтирования
    void start();
    void stop();
    void restart();

  private:
    void main_task(void);

    std::string logname;
    std::string mount_root;

    std::thread main_thread;
    
    std::atomic_bool is_running;
    bool is_daemon_mode;

    struct udev *udev;
    struct udev_device *dev;
    struct udev_monitor *mon;   

    int fd;
};