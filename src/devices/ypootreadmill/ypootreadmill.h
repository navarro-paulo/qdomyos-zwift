#ifndef YPOOTREADMILL_H
#define YPOOTREADMILL_H

#include "treadmill.h"
#include "ypoom5control.h"
#include "ypoom5distance.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>

// YPTM-U and YP-M1552 share the unlock/control protocol but use different incline units.
// YPTM-U names are "YPTM-U" + the last three MAC bytes for both the M5 and the M5 MAX (and rebrands
// such as the MegaGym Everest 49), so the model cannot be told from the name: the incline ceiling is
// read from the FTMS Supported Range characteristics (2AD4/2AD5) and only widens the M5 defaults.
class ypootreadmill : public treadmill {
    Q_OBJECT
  public:
    ypootreadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    void stopWhenIdle() override;
    VirtualTreadmillControl virtualTreadmillControl() const override;
    double minStepInclination() override { return 1.0; }
    double minStepSpeed() override { return 0.1; }
    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;

  signals:
    void disconnected();
    void debug(QString message);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private:
    void btinit();
    void updateYpooM5Control();
    void forceSpeed(double speed);
    void forceIncline(double incline);
    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                             uint8_t *data, uint8_t size, QString info);
    void processTelemetry(const QByteArray &data, qint64 now);
    void applyModelRanges();
    bool applySupportedRange(const QBluetoothUuid &uuid, const QByteArray &value);
    static QByteArray encodeRange(double min, double max, double step, double scale);
    friend class YpooTreadmillTestPeer;

    QTimer refresh;
    QLowEnergyService *gattFTMSService = nullptr;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyCharacteristic gattWriteCharUnlock;
    bool noHeartService;
    bool YP_M1552 = false;
    bool YPTM_U = false;
    double speedMin = 1.0;
    double speedMax = 18.8;
    double inclineMin = -6.0;
    double inclineMax = 15.0;
    bool initDone = false;
    bool initRequest = false;
    unsigned notificationSubscribed = 0;
    bool ypooM5WritePending = false;
    bool ypooM5ControlGranted = false;
    QElapsedTimer ypooM5Clock;
    YpooM5Control ypooM5Control;
    YpooM5Distance ypooM5Distance;
    qint64 lastMetricsUpdate = -1;

  private slots:
    void update();
    void serviceScanDone();
    void stateChanged(QLowEnergyService::ServiceState state);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void changeInclinationRequested(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
};

#endif // YPOOTREADMILL_H
