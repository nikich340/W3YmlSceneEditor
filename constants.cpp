#include "constants.h"

const int CONSTANTS::IsNavigationLine = 0;

const QColor CONSTANTS::colorSceneGradient0 = QColor(235, 235, 224), CONSTANTS::colorSceneGradient1(195, 195, 162);
const QColor CONSTANTS::colorSceneGradient00(235, 235, 224), CONSTANTS::colorSceneGradient11(195, 20, 162);
const QColor CONSTANTS::colorSectionNormal(240, 255, 112), CONSTANTS::colorSectionIncomplete(255, 122, 122);
const QColor CONSTANTS::colorSocketNormal(0, 220, 220), CONSTANTS::colorSocketLabel(255, 51, 133);
const QColor CONSTANTS::colorEdgeNormal(0, 0, 100), CONSTANTS::colorEdgeChange(128, 0, 50), CONSTANTS::colorEdgeHighlight(0, 90, 20);

const QVector<QColor> CONSTANTS::colorDgViewActors = {
    // 0 - CHOICE
    // 3 - PAUSE
    CONSTANTS::QColorLight(1, 15), CONSTANTS::QColorLight(4, 15), CONSTANTS::QColorLight(6, 15), CONSTANTS::QColorLight(8, 15),
    CONSTANTS::QColorLight(10, 15), CONSTANTS::QColorLight(12, 15), CONSTANTS::QColorLight(14, 15), CONSTANTS::QColorLight(2, 15),
    CONSTANTS::QColorLight(5, 15), CONSTANTS::QColorLight(7, 15), CONSTANTS::QColorLight(9, 15), CONSTANTS::QColorLight(11, 15),
    CONSTANTS::QColorLight(13, 15)
};

const QColor CONSTANTS::colorDgViewPause = CONSTANTS::QColorLight(3, 15), CONSTANTS::colorDgViewChoice = CONSTANTS::QColorLight(0, 15);

const QString CONSTANTS::REPO_TEMP = "_repo_temp_";

const double CONSTANTS::DIAMETER = 6.0;
const double CONSTANTS::HEIGHT = 60.0;
const double CONSTANTS::WIDTH = 125.0;
const double CONSTANTS::SCENE_HEIGHT = 1250.0;
const double CONSTANTS::SCENE_WIDTH = 5000.0;

const double CONSTANTS::SHOT_SCENE_HEIGHT = 2000.0;
const double CONSTANTS::SHOT_SCENE_WIDTH = 3000.0;
const double CONSTANTS::SHOT_SECOND = 100;
const double CONSTANTS::SHOT_ROW = 25;
const double CONSTANTS::SHOT_DG_HEIGHT = 70.0;
const double CONSTANTS::SHOT_LABEL_HEIGHT = 80.0;
const double CONSTANTS::SHOT_LABEL_PEN_WIDTH = 1.5;
const double CONSTANTS::SHOT_LABEL_WIDTH = 150.0;

const QHash<QString, EShotActionType> CONSTANTS::stringToEShotAction = {
    { "unknown!",               EShotUnknown            },
    { "cam",                    EShotCam                },
    { "cam.blend.start",        EShotCamBlendStart      },
    { "cam.blend.key",          EShotCamBlendKey        },
    { "cam.blend.end",          EShotCamBlendEnd        },
    { "cam.blend.togame",       EShotCamBlendTogame     },
    { "env.blendin",            EShotEnvBlendIn         },
    { "env.blendout",           EShotEnvBlendOut        },
    { "fade.in",                EShotFadeIn             },
    { "fade.out",               EShotFadeOut            },
    { "world.addfact",          EShotWorldAddfact       },
    { "world.weather",          EShotWorldWeather       },
    { "world.effect.start",     EShotWorldEffectStart   },
    { "world.effect.stop",      EShotWorldEffectStop    },
    { "actor.anim",             EShotActorAnim          },
    { "anim",                   EShotActorAnim          },
    { "actor.anim.additive",    EShotActorAnimAdditive  },
    { "anim.additive",          EShotActorAnimAdditive  },
    { "actor.pose",             EShotActorAnimPose      },
    { "actor.anim.mimic",       EShotActorMimicAnim     },
    { "anim.mimic",             EShotActorMimicAnim     },
    { "actor.mimic",            EShotActorMimicPose     },
    { "actor.placement",        EShotActorPlacement     },
    { "actor.placement.start",  EShotActorPlacementStart},
    { "actor.placement.key",    EShotActorPlacementKey  },
    { "actor.placement.end",    EShotActorPlacementEnd  },
    { "actor.gamestate",        EShotActorGamestate     },
    { "actor.lookat",           EShotActorLookat        },
    { "actor.scabbard.show",    EShotActorScabbardShow  },
    { "actor.scabbard.hide",    EShotActorScabbardHide  },
    { "actor.show",             EShotActorShow          },
    { "actor.hide",             EShotActorHide          },
    { "actor.effect.start",     EShotActorEffectStart   },
    { "actor.effect.stop",      EShotActorEffectStop    },
    { "actor.sound",            EShotActorSound         },
    { "actor.appearance",       EShotActorAppearance    },
    { "actor.equip.right",      EShotActorEquipRight    },
    { "actor.equip.left",       EShotActorEquipLeft     },
    { "actor.unequip.right",    EShotActorUnequipRight  },
    { "actor.unequip.left",     EShotActorUnequipLeft   },
    { "prop.show",              EShotPropShow           },
    { "prop.hide",              EShotPropHide           },
    { "prop.effect.start",      EShotPropEffectStart    },
    { "prop.effect.stop",       EShotPropEffectStop     },
    { "prop.placement",         EShotPropPlacement      },
    { "prop.placement.start",   EShotPropPlacementStart },
    { "prop.placement.key",     EShotPropPlacementKey   },
    { "prop.placement.end",     EShotPropPlacementEnd   }
};
const QHash<EShotActionType, QString> CONSTANTS::EShotActionToString = {
    { EShotUnknown,             "unknown!"              },
    { EShotCam,                 "cam"                   },
    { EShotCamBlendStart,       "cam.blend.start"       },
    { EShotCamBlendKey,         "cam.blend.key"         },
    { EShotCamBlendEnd,         "cam.blend.end"         },
    { EShotCamBlendTogame,      "cam.blend.togame"      },
    { EShotEnvBlendIn,          "env.blendin"           },
    { EShotEnvBlendOut,         "env.blendout"          },
    { EShotFadeIn,              "fade.in"               },
    { EShotFadeOut,             "fade.out"              },
    { EShotWorldAddfact,        "world.addfact"         },
    { EShotWorldWeather,        "world.weather"         },
    { EShotWorldEffectStart,    "world.effect.start"    },
    { EShotWorldEffectStop,     "world.effect.stop"     },
    { EShotActorAnim,           "actor.anim"            },
    { EShotActorAnimAdditive,   "actor.anim.additive"   },
    { EShotActorAnimPose,       "actor.pose"            },
    { EShotActorMimicAnim,      "actor.anim.mimic"      },
    { EShotActorMimicPose,      "actor.mimic"           },
    { EShotActorPlacement,      "actor.placement"       },
    { EShotActorPlacementStart, "actor.placement.start" },
    { EShotActorPlacementKey,   "actor.placement.key"   },
    { EShotActorPlacementEnd,   "actor.placement.end"   },
    { EShotActorGamestate,      "actor.gamestate"       },
    { EShotActorLookat,         "actor.lookat"          },
    { EShotActorScabbardShow,   "actor.scabbard.show"   },
    { EShotActorScabbardHide,   "actor.scabbard.hide"   },
    { EShotActorShow,           "actor.show"            },
    { EShotActorHide,           "actor.hide"            },
    { EShotActorEffectStart,    "actor.effect.start"    },
    { EShotActorEffectStop,     "actor.effect.stop"     },
    { EShotActorSound,          "actor.sound"           },
    { EShotActorAppearance,     "actor.appearance"      },
    { EShotActorEquipRight,     "actor.equip.right"     },
    { EShotActorEquipLeft,      "actor.equip.left"      },
    { EShotActorUnequipRight,   "actor.unequip.right"   },
    { EShotActorUnequipLeft,    "actor.unequip.left"    },
    { EShotPropShow,            "prop.show"             },
    { EShotPropHide,            "prop.hide"             },
    { EShotPropEffectStart,     "prop.effect.start"     },
    { EShotPropEffectStop,      "prop.effect.stop"      },
    { EShotPropPlacement,       "prop.placement"        },
    { EShotPropPlacementStart,  "prop.placement.start"  },
    { EShotPropPlacementKey,    "prop.placement.key"    },
    { EShotPropPlacementEnd,    "prop.placement.end"    }
};
const QHash<EShotActionType, int> CONSTANTS::EShotActionToGroupNum = {
    { EShotCam,                 0   },
    { EShotCamBlendStart,       0   },
    { EShotCamBlendKey,         0   },
    { EShotCamBlendEnd,         0   },
    { EShotCamBlendTogame,      0   },
    { EShotEnvBlendIn,          1   },
    { EShotEnvBlendOut,         1   },
    { EShotFadeIn,              1   },
    { EShotFadeOut,             1   },
    { EShotWorldAddfact,        1   },
    { EShotWorldWeather,        1   },
    { EShotWorldEffectStart,    1   },
    { EShotWorldEffectStop,     1   },
    { EShotActorAnim,           1   },
    { EShotActorAnimAdditive,   2   },
    { EShotActorAnimPose,       2   },
    { EShotActorMimicAnim,      3   },
    { EShotActorMimicPose,      3   },
    { EShotActorPlacement,      5   },
    { EShotActorPlacementStart, 5   },
    { EShotActorPlacementKey,   5   },
    { EShotActorPlacementEnd,   5   },
    { EShotActorGamestate,      0   },
    { EShotActorLookat,         4   },
    { EShotActorScabbardShow,   0   },
    { EShotActorScabbardHide,   0   },
    { EShotActorShow,           0   },
    { EShotActorHide,           0   },
    { EShotActorEffectStart,    6   },
    { EShotActorEffectStop,     6   },
    { EShotActorSound,          6   },
    { EShotActorAppearance,     0   },
    { EShotActorEquipRight,     0   },
    { EShotActorEquipLeft,      0   },
    { EShotActorUnequipRight,   0   },
    { EShotActorUnequipLeft,    0   },
    { EShotPropShow,            0   },
    { EShotPropHide,            0   },
    { EShotPropEffectStart,     2   },
    { EShotPropEffectStop,      2   },
    { EShotPropPlacement,       1   },
    { EShotPropPlacementStart,  1   },
    { EShotPropPlacementKey,    1   },
    { EShotPropPlacementEnd,    1   }
};
const QVector<QVector<EShotActionType>> CONSTANTS::GroupNumToEShotActionShared = {
    { /* 0 */ EShotCam, EShotCamBlendStart, EShotCamBlendKey, EShotCamBlendEnd, EShotCamBlendTogame },
    { /* 1 */ EShotEnvBlendIn, EShotEnvBlendOut, EShotFadeIn, EShotFadeOut, EShotWorldAddfact, EShotWorldWeather, EShotWorldEffectStart, EShotWorldEffectStop }
};
const QVector<QVector<EShotActionType>> CONSTANTS::GroupNumToEShotActionActor = {
    { /* 0 */ EShotActorShow, EShotActorHide, EShotActorAppearance, EShotActorEquipRight,
      EShotActorEquipLeft, EShotActorUnequipRight, EShotActorUnequipLeft, EShotActorGamestate,
      EShotActorScabbardShow, EShotActorScabbardHide },
    { /* 1 */ EShotActorAnim },
    { /* 2 */ EShotActorAnimAdditive, EShotActorAnimPose },
    { /* 3 */ EShotActorMimicAnim, EShotActorMimicPose },
    { /* 4 */ EShotActorLookat, EShotActorMimicPose },
    { /* 5 */ EShotActorPlacement, EShotActorPlacementStart, EShotActorPlacementKey, EShotActorPlacementEnd },
    { /* 6 */ EShotActorEffectStart, EShotActorEffectStop, EShotActorSound },
};
const QVector<QVector<EShotActionType>> CONSTANTS::GroupNumToEShotActionProp = {
    { /* 0 */ EShotPropShow, EShotPropHide },
    { /* 1 */ EShotPropPlacement, EShotPropPlacementStart, EShotPropPlacementKey, EShotPropPlacementEnd },
    { /* 2 */ EShotPropEffectStart, EShotPropEffectStop }
};
const QSet<EShotActionType> CONSTANTS::EShotActionsShared = {
    EShotCam,
    EShotCamBlendStart,
    EShotCamBlendKey,
    EShotCamBlendEnd,
    EShotCamBlendTogame,
    EShotEnvBlendIn,
    EShotEnvBlendOut,
    EShotFadeIn,
    EShotFadeOut,
    EShotWorldAddfact,
    EShotWorldWeather,
    EShotWorldEffectStart,
    EShotWorldEffectStop
};

const QStringList CONSTANTS::MIMICS_LIST = { "Afraid", "Very afraid", "Aggressive", "Bursting_anger", "Confident", "Contempt", "Cry",
                                  "Determined", "Disgusted", "Drunk", "Focused", "Happy", "Very happy", "Nervous", "Neutral",
                                             "Proud", "Sad", "Sceptic", "Seducing", "Sleeping", "Surprised", "Surprised Shocked", "NO ANIM" };

