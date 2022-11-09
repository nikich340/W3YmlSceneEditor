#ifndef YMLShotActions_H
#define YMLShotActions_H
#include <QString>
#include <QWidget>
#include <yaml-cpp/yaml.h>
#include "YmlStructs.h"
#include "constants.h"

class SA_Base {
protected:
    bool m_isExtended = false;
    int m_assetID = -1;
    double m_start = 0.0;
    sceneInfo* m_pSG = nullptr;
    QWidget* m_pWidget = nullptr;
    QString m_actionTypeStr;
    QSet<QString> m_optionValues;  // contains optional value names checked to be included in yml
    YAML::Node m_ymlNode;

public:
    static SA_Base* createShotAction(const EShotActionType SA_Type, sceneInfo* pSG, double start = 0.0);
    static QString shortDescription(const EShotActionType SA_Type);
    bool operator<(const SA_Base& sa) const;
    int assetID() { return m_assetID; }
    double start() { return m_start; }
    void setStart(double newStart);
    const YAML::Node ymlNode() const;
    const QString& actionTypeStr() const;

    virtual EShotActionType actionType() { return EShotUnknown; }
    virtual void initDefaults(int assetID = -1) {}
    virtual bool importYmlNode(const YAML::Node& paramNode, QString& outMessage);
    virtual void updateYmlNode();
    /** loads data from widget + updateYmlNode internally **/
    virtual void importWidget();
    /** exports data to widget **/
    virtual void exportWidget(QWidget* pWidget);
};

/* CAMERA */
class SA_Cam : public SA_Base {
protected:
    int v_camNameID;

public:
    using super = SA_Base;
    EShotActionType actionType() override { return EShotCam; }
    virtual void initDefaults(int assetID = -1);
    virtual bool importYmlNode(const YAML::Node& paramNode, QString& outMessage);
    virtual void updateYmlNode();
    virtual void importWidget();
    virtual void exportWidget(QWidget* pWidget);
};

class SA_CamBlendKey : public SA_Cam {
public:
    using super = SA_Cam;
    EShotActionType actionType() override { return EShotCamBlendKey; }
};

class SA_CamBlendTogame : public SA_Cam {
public:
    using super = SA_Cam;
    EShotActionType actionType() override { return EShotCamBlendTogame; }
};

class SA_CamBlendStart : public SA_Cam {
protected:
    QString v_camEase = "smooth";

public:
    using super = SA_Cam;
    EShotActionType actionType() override { return EShotCamBlendStart; }
    virtual bool importYmlNode(const YAML::Node& paramNode, QString& outMessage);
    virtual void updateYmlNode();
    virtual void importWidget();
    virtual void exportWidget(QWidget* pWidget);
};

class SA_CamBlendEnd : public SA_CamBlendStart {
public:
    using super = SA_CamBlendStart;
    EShotActionType actionType() override { return EShotCamBlendEnd; }
};

/* ENV */
class SA_EnvBlendIn : public SA_Base {
protected:
    QString v_envPath = "dlc/bob/data/environment/definitions/sunny/sunny_bob_v7_4.env";
    double v_blendTime = 0.0;
    double v_blendFactor = 1.0;
    int v_priority = 1000;

public:
    using super = SA_Base;
    EShotActionType actionType() override { return EShotEnvBlendIn; }
    virtual bool importYmlNode(const YAML::Node& paramNode, QString& outMessage);
    virtual void updateYmlNode();
    virtual void importWidget();
    virtual void exportWidget(QWidget* pWidget);
};

class SA_EnvBlendOut : public SA_EnvBlendIn {
    using super = SA_EnvBlendIn;
    EShotActionType actionType() override { return EShotEnvBlendOut; }
};

#endif // YMLShotActions_H
