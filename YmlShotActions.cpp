#include <QDebug>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include "YmlShotActions.h"

/* ShotActionBase */
const YAML::Node SA_Base::ymlNode() const
{
    return YAML::Clone(m_ymlNode);
}

void SA_Base::setStart(double newStart)
{
    m_start = newStart;
}

const QString& SA_Base::actionTypeStr() const
{
    return m_actionTypeStr;
}

SA_Base* SA_Base::createShotAction(const EShotActionType SA_Type, sceneInfo* pSG, const double start)
{
    SA_Base* ret = nullptr;
    switch (SA_Type) {
        /* SHARED */
        case EShotCam:
           ret = new SA_Cam;
           break;
        case EShotCamBlendStart:
            ret = new SA_CamBlendStart;
            break;
        case EShotCamBlendKey:
            ret = new SA_CamBlendKey;
            break;
        case EShotCamBlendEnd:
            ret = new SA_CamBlendEnd;
            break;
        case EShotCamBlendTogame:
            ret = new SA_CamBlendTogame;
            break;
        case EShotEnvBlendIn:
            ret = new SA_EnvBlendIn;
            break;
        case EShotEnvBlendOut:
            ret = new SA_EnvBlendOut;
            break;
        /*case EShotFadeIn:
            ret = new SA_FadeIn;
            break;
        case EShotFadeOut:
            ret = new SA_FadeOut;
            break;
        case EShotWorldAddfact:
            ret = new SA_WorldAddfact;
            break;
        case EShotWorldWeather:
            ret = new SA_WorldWeather;
            break;
        case EShotWorldEffectStart:
            ret = new SA_WorldEffectStart;
            break;
        case EShotWorldEffectStop:
            ret = new SA_WorldEffectStop;
            break;*/
        /* ACTOR */
        /*case EShotActorAnim:
            ret = new SA_ActorAnim;
            break;
        case EShotActorAnimAdditive:
            ret = new SA_ActorAnimAdditive;
            break;
        case EShotActorAnimPose:
            ret = new SA_ActorAnimPose;
            break;
        case EShotActorMimicAnim:
            ret = new SA_ActorMimicAnim;
            break;
        case EShotActorMimicPose:
            ret = new SA_ActorMimicPose;
            break;
        case EShotActorPlacement:
            ret = new SA_ActorPlacement;
            break;
        case EShotActorPlacementStart:
            ret = new SA_ActorPlacementStart;
            break;
        case EShotActorPlacementKey:
            ret = new SA_ActorPlacementKey;
            break;
        case EShotActorPlacementEnd:
            ret = new SA_ActorPlacementEnd;
            break;
        case EShotActorGamestate:
            ret = new SA_ActorGamestate;
            break;
        case EShotActorLookat:
            ret = new SA_ActorLookat;
            break;
        case EShotActorScabbardShow:
            ret = new SA_ActorScabbardShow;
            break;
        case EShotActorScabbardHide:
            ret = new SA_ActorScabbardHide;
            break;
        case EShotActorEffectStart:
            ret = new SA_ActorEffectStart;
            break;
        case EShotActorEffectStop:
            ret = new SA_ActorEffectStop;
            break;
        case EShotActorSound:
            ret = new SA_ActorSound;
            break;
        case EShotActorAppearance:
            ret = new SA_ActorAppearance;
            break;
        case EShotActorEquipRight:
            ret = new SA_ActorEquipRight;
            break;
        case EShotActorEquipLeft:
            ret = new SA_ActorEquipLeft;
            break;
        case EShotActorUnequipRight:
            ret = new SA_ActorUnequipRight;
            break;
        case EShotActorUnequipLeft:
            ret = new SA_ActorUnequipLeft;
            break;*/
        /* PROP */
        /*case EShotPropShow:
            ret = new SA_PropShow;
            break;
        case EShotPropHide:
            ret = new SA_PropHide;
            break;
        case EShotPropPlacement:
            ret = new SA_PropPlacement;
            break;
        case EShotPropPlacementStart:
            ret = new SA_PropPlacementStart;
            break;
        case EShotPropPlacementKey:
            ret = new SA_PropPlacementKey;
            break;
        case EShotPropPlacementEnd:
            ret = new SA_PropPlacementEnd;
            break;
        case EShotPropEffectStart:
            ret = new SA_PropEffectStart;
            break;
        case EShotPropEffectStop:
            ret = new SA_PropEffectStop;
            break;*/
        default:
           qc << QString("%1: unknown type = %2").arg(Q_FUNC_INFO).arg(CONSTANTS::EShotActionToString[SA_Type]);
           ret = new SA_Base;
           break;
    }
    ret->m_pSG = pSG;
    ret->m_start = start;
    ret->m_actionTypeStr = CONSTANTS::EShotActionToString[ret->actionType()];
    return ret;
}

QString SA_Base::shortDescription(const EShotActionType SA_Type)
{
    switch (SA_Type) {
        /* SHARED */
        case EShotCam:
            return "Changes scene camera. Can't be used during cam blendings.";
        case EShotCamBlendStart:
            return "Starts camera blending at specified camera to next cam.blend.key/.end. <br>Optionally ease in is smoothed.";
        case EShotCamBlendKey:
            return "Defines scene camera key (intermediate point) for blending. <br>Always bezier smoothed.";
        case EShotCamBlendEnd:
            return "Ends camera blending from previous cam.blend.start/.key at specified camera. <br>Optionally ease out is smoothed.";
        case EShotCamBlendTogame:
            return "Blends the camera seamlessly to the normal user controlled gamplay camera (~2s). <br>You can't use another cam events after this. Consider using at the same time with actor.gamestate for actors.";
        case EShotEnvBlendIn:
            return "Blends to to specified .env (use absolute path).";
        case EShotEnvBlendOut:
            return "Blends out from specified .env (use absolute path). <br>Makes sense after env.blendin with the same path.";
        case EShotFadeIn:
            return "-";
        case EShotFadeOut:
            return "-";
        case EShotWorldAddfact:
            return "-";
        case EShotWorldWeather:
            return "-";
        case EShotWorldEffectStart:
            return "-";
        case EShotWorldEffectStop:
            return "-";

        /* ACTOR */
        case EShotActorAnim:
            return "-";
        case EShotActorAnimAdditive:
            return "-";
        case EShotActorAnimPose:
            return "-";
        case EShotActorMimicAnim:
            return "-";
        case EShotActorMimicPose:
            return "-";
        case EShotActorPlacement:
            return "-";
        case EShotActorPlacementStart:
            return "-";
        case EShotActorPlacementKey:
            return "-";
        case EShotActorPlacementEnd:
            return "-";
        case EShotActorGamestate:
            return "-";
        case EShotActorLookat:
            return "-";
        case EShotActorScabbardShow:
            return "-";
        case EShotActorScabbardHide:
            return "-";
        case EShotActorEffectStart:
            return "-";
        case EShotActorEffectStop:
            return "-";
        case EShotActorSound:
            return "-";
        case EShotActorAppearance:
            return "-";
        case EShotActorEquipRight:
            return "-";
        case EShotActorEquipLeft:
            return "-";
        case EShotActorUnequipRight:
            return "-";
        case EShotActorUnequipLeft:
            return "-";

        /* PROP */
        case EShotPropShow:
            return "-";
        case EShotPropHide:
            return "-";
        case EShotPropPlacement:
            return "-";
        case EShotPropPlacementStart:
            return "-";
        case EShotPropPlacementKey:
            return "-";
        case EShotPropPlacementEnd:
            return "-";
        case EShotPropEffectStart:
            return "-";
        case EShotPropEffectStop:
            return "-";
        default:
           qc << QString("%1: unknown type = %2").arg(Q_FUNC_INFO).arg(CONSTANTS::EShotActionToString[SA_Type]);
           return "Abstract action type. You are not supposed to see this!";
    }
}

bool SA_Base::operator<(const SA_Base& sa) const
{
    return (m_start < sa.m_start);
}

bool SA_Base::importYmlNode(const YAML::Node& paramNode, QString& outMessage)
{
    m_isExtended = paramNode.IsMap();
    if (paramNode.IsMap()) {
        if (!paramNode[".@pos"]|| !paramNode[".@pos"].IsSequence()) {
            outMessage = QString("%1: param values are map, but .@pos not found or is not a sequence.").arg(Q_FUNC_INFO);
            return false;
        }
        if (paramNode[".@pos"].size() < 2) {
            outMessage = QString("%1: .@pos contains less than 2 values").arg(Q_FUNC_INFO);
            return false;
        }
        m_start = paramNode[".@pos"][0].as<double>();
    } else if (paramNode.IsSequence()) {
        if (paramNode.size() < 2) {
            outMessage = QString("%1: sequence contains less than 2 values").arg(Q_FUNC_INFO);
            return false;
        }
        m_start = paramNode[0].as<double>();
        qd << QString("%1: start = %2").arg(Q_FUNC_INFO).arg(m_start);
    } else {
        outMessage = QString("%1: param values type is %2").arg(Q_FUNC_INFO).arg(paramNode.Type());
        return false;
    }

    return true;
}

void SA_Base::updateYmlNode()
{
    m_ymlNode.reset();
    if (!m_isExtended) {
        m_ymlNode[ m_actionTypeStr ] = YAML::Node();
        m_ymlNode[ m_actionTypeStr ].SetStyle(YAML::EmitterStyle::Flow);
        m_ymlNode[ m_actionTypeStr ].push_back(start());
    } else {
        m_ymlNode[ m_actionTypeStr ] = YAML::Node();
        m_ymlNode[ m_actionTypeStr ].SetStyle(YAML::EmitterStyle::Block);
        m_ymlNode[ m_actionTypeStr ][ ".@pos" ] = YAML::Node();
        m_ymlNode[ m_actionTypeStr ][ ".@pos" ].SetStyle(YAML::EmitterStyle::Flow);
        m_ymlNode[ m_actionTypeStr ][ ".@pos" ].push_back(start());
    }
}

void SA_Base::importWidget()
{
    if (m_pWidget == nullptr) {
        qc << QString("%1: m_pWidget is NULL.").arg(Q_FUNC_INFO);
        return;
    }
    QDoubleSpinBox* pStartSpin = m_pWidget->findChild<QDoubleSpinBox*>("m_startSpin");
    if (pStartSpin == nullptr) {
        qc << QString("%1: pStartSpin is NULL.").arg(Q_FUNC_INFO);
        return;
    }
    m_isExtended = false;  // always reset, set to true if needed in children
    m_start = pStartSpin->value();
}

void SA_Base::exportWidget(QWidget* pWidget)
{
    m_pWidget = pWidget;
    if (m_pWidget == nullptr) {
        qc << QString("%1: m_pWidget is NULL.").arg(Q_FUNC_INFO);
        return;
    }
    QDoubleSpinBox* pStartSpin = m_pWidget->findChild<QDoubleSpinBox*>("m_startSpin");
    if (pStartSpin == nullptr) {
        qc << QString("%1: pStartSpin is NULL.").arg(Q_FUNC_INFO);
        return;
    }
    pStartSpin->setValue(m_start);
    QSlider* pStartSlider = m_pWidget->findChild<QSlider*>("m_startSlider");
    int startInt = m_start * 1000.0;
    pStartSlider->setValue(startInt);
}

/* ShotActionCam */
void SA_Cam::initDefaults(int assetID)
{
    super::initDefaults(assetID);
    v_camNameID = m_pSG->getIDAny(ERepoCameras);
}

bool SA_Cam::importYmlNode(const YAML::Node& paramNode, QString& outMessage)
{
    if ( !super::importYmlNode(paramNode, outMessage) )
        return false;

    if (m_isExtended) {
        outMessage = QString("%1: cam action must be a sequence").arg(Q_FUNC_INFO);
        return false;
    } else {
        QString name = paramNode[1].as<QString>();
        if ( !m_pSG->hasName(ERepoCameras, name) ) {
            outMessage = QString("%1: cam %2 not found in repo").arg(Q_FUNC_INFO).arg(name);
            return false;
        }
        v_camNameID = m_pSG->getID(ERepoCameras, name);
    }
    return true;
}

void SA_Cam::updateYmlNode()
{
    super::updateYmlNode();
    if (!m_isExtended) {
        m_ymlNode[ m_actionTypeStr ].push_back( m_pSG->getName(v_camNameID) );
    } else {
        m_ymlNode[ m_actionTypeStr ][ ".@pos" ].push_back( m_pSG->getName(v_camNameID) );
    }
}

void SA_Cam::importWidget()
{
    super::importWidget();
    QComboBox* pNamesBox = m_pWidget->findChild<QComboBox*>("v_camName");
    if (pNamesBox->currentIndex() < 0)
        return;
    QString name = pNamesBox->currentText();
    if (!m_pSG->hasName(ERepoCameras, name)) {
        qc << QString("%1: camera %2 not found in repo.").arg(Q_FUNC_INFO).arg(name);
        return;
    }
    v_camNameID = m_pSG->getID(ERepoCameras, name);
}

void SA_Cam::exportWidget(QWidget* pWidget)
{
    super::exportWidget(pWidget);
    QComboBox* pNamesBox = m_pWidget->findChild<QComboBox*>("v_camName");
    pNamesBox->clear();
    QSet<QString> allCamNames;
    for (auto camID : m_pSG->cameras.keys()) {
        allCamNames.insert(m_pSG->getName(camID));
    }
    pNamesBox->addItems( allCamNames.toList() );
    qDebug() << "exportWidget: " << m_pSG->getName(v_camNameID);
    if (m_pSG != nullptr)
        pNamesBox->setCurrentText( m_pSG->getName(v_camNameID) );
}

bool SA_CamBlendStart::importYmlNode(const YAML::Node& paramNode, QString& outMessage)
{
    if ( !super::importYmlNode(paramNode, outMessage) )
        return false;
    if ( paramNode.size() > 2 ) {
        v_camEase = paramNode[2].as<QString>();
        m_optionValues.insert("v_camEase");
        if (v_camEase != "rapid" && v_camEase != "smooth") {
            outMessage = QString("%1: cam %2: unknown ease type = %3").arg(Q_FUNC_INFO).arg(m_pSG->getName(v_camNameID)).arg(v_camEase);
            return false;
        }
    }
    return true;
}

void SA_CamBlendStart::updateYmlNode()
{
    super::updateYmlNode();
    if (!m_isExtended) {
        if (m_optionValues.contains("v_camEase"))
            m_ymlNode[ m_actionTypeStr ].push_back( v_camEase );
    } else {
        if (m_optionValues.contains("v_camEase"))
            m_ymlNode[ m_actionTypeStr ][ ".@pos" ].push_back( v_camEase );
    }
}

void SA_CamBlendStart::importWidget()
{
    super::importWidget();
    QComboBox* pEaseBox = m_pWidget->findChild<QComboBox*>("v_camEase");
    if (pEaseBox->isEnabled()) {
        m_optionValues.insert("v_camEase");
        v_camEase = pEaseBox->currentText();
    } else {
        m_optionValues.remove("v_camEase");
    }
}

void SA_CamBlendStart::exportWidget(QWidget* pWidget)
{
    super::exportWidget(pWidget);
    QCheckBox* pEaseCheck = m_pWidget->findChild<QCheckBox*>("v_camEaseCheck");
    pEaseCheck->setChecked(m_optionValues.contains("v_camEase"));
    QComboBox* pEaseBox = m_pWidget->findChild<QComboBox*>("v_camEase");
    pEaseBox->setCurrentText(v_camEase);
}

bool SA_EnvBlendIn::importYmlNode(const YAML::Node& paramNode, QString& outMessage)
{
    if ( !super::importYmlNode(paramNode, outMessage) )
        return false;
    if (!m_isExtended) {
        v_envPath = paramNode[1].as<QString>();
        if (paramNode.size() > 2) {
            v_blendTime = paramNode[2].as<double>();
            m_optionValues.insert("v_blendTime");
        }
    } else {
        v_envPath = paramNode[".@pos"][1].as<QString>();
        if (paramNode[".@pos"].size() > 2) {
            v_blendTime = paramNode[".@pos"][2].as<double>();
            m_optionValues.insert("v_blendTime");
        }

        if (paramNode["priority"]) {
            v_priority = paramNode["priority"].as<int>();
            m_optionValues.insert("v_priority");
        }
        if (paramNode["blendFactor"]) {
            v_blendFactor = paramNode["blendFactor"].as<double>();
            m_optionValues.insert("v_blendFactor");
        }
    }

    return true;
}

void SA_EnvBlendIn::updateYmlNode()
{
    super::updateYmlNode();
    if (!m_isExtended) {
        m_ymlNode[ m_actionTypeStr ].push_back( v_envPath );
        if (m_optionValues.contains("v_blendTime"))
            m_ymlNode[ m_actionTypeStr ].push_back( v_blendTime );
    } else {
        m_ymlNode[ m_actionTypeStr ][ ".@pos" ].push_back( v_envPath );
        if (m_optionValues.contains("v_blendTime"))
            m_ymlNode[ m_actionTypeStr ][ ".@pos" ].push_back( v_blendTime );
        if (m_optionValues.contains("v_blendFactor"))
            m_ymlNode[ "blendFactor" ] = v_blendFactor;
        if (m_optionValues.contains("v_priority"))
            m_ymlNode[ "priority" ] = v_priority;
    }
}

void SA_EnvBlendIn::importWidget()
{
    super::importWidget();
    QLineEdit* pEnvPathLine = m_pWidget->findChild<QLineEdit*>("v_envPath");
    v_envPath = pEnvPathLine->text();

    QDoubleSpinBox* pBlendTimeSpin = m_pWidget->findChild<QDoubleSpinBox*>("v_blendTime");
    if (pBlendTimeSpin->isEnabled()) {
        m_optionValues.insert("v_blendTime");
        v_blendTime = pBlendTimeSpin->value();
    } else {
        m_optionValues.remove("v_blendTime");
    }

    QDoubleSpinBox* pBlendFactorSpin = m_pWidget->findChild<QDoubleSpinBox*>("v_blendFactor");
    if (pBlendFactorSpin->isEnabled()) {
        m_isExtended = true;
        m_optionValues.insert("v_blendFactor");
        v_blendTime = pBlendFactorSpin->value();
    } else {
        m_optionValues.remove("v_blendFactor");
    }

    QSpinBox* pPrioritySpin = m_pWidget->findChild<QSpinBox*>("v_priority");
    if (pPrioritySpin->isEnabled()) {
        m_isExtended = true;
        m_optionValues.insert("v_priority");
        v_blendTime = pPrioritySpin->value();
    } else {
        m_optionValues.remove("v_priority");
    }
}

void SA_EnvBlendIn::exportWidget(QWidget* pWidget)
{
    super::exportWidget(pWidget);
    QCheckBox* pBlendTimeSpinCheck = m_pWidget->findChild<QCheckBox*>("v_blendTimeCheck");
    pBlendTimeSpinCheck->setChecked(m_optionValues.contains("v_blendTime"));
    QDoubleSpinBox* pBlendTimeSpin = m_pWidget->findChild<QDoubleSpinBox*>("v_blendTime");
    pBlendTimeSpin->setValue(v_blendTime);

    QCheckBox* pBlendFactorCheck = m_pWidget->findChild<QCheckBox*>("v_blendTimeCheck");
    pBlendFactorCheck->setChecked(m_optionValues.contains("v_blendFactor"));
    QDoubleSpinBox* pBlendFactorSpin = m_pWidget->findChild<QDoubleSpinBox*>("v_blendFactor");
    pBlendFactorSpin->setValue(v_blendFactor);

    QCheckBox* pPriorityCheck = m_pWidget->findChild<QCheckBox*>("v_priorityCheck");
    pPriorityCheck->setChecked(m_optionValues.contains("v_priority"));
    QSpinBox* pPrioritySpin = m_pWidget->findChild<QSpinBox*>("v_priority");
    pPrioritySpin->setValue(v_priority);
}
