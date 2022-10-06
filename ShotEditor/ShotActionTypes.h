#pragma once
#ifndef SHOTACTIONTYPES_H
#define SHOTACTIONTYPES_H

#include <QHash>
#include <QString>
#include <QColor>

enum EShotActionType {
    EShotUnknown,
    EShotCam,
    EShotCamBlendStart,
    EShotCamBlendKey,
    EShotCamBlendEnd,
    EShotCamBlendTogame,
    EShotActorAnim,
    EShotActorAnimAdditive,
    EShotActorAnimPose,
    EShotActorMimicAnim,
    EShotActorMimicPose,
    EShotActorPlacement,
    EShotActorPlacementStart,
    EShotActorPlacementKey,
    EShotActorPlacementEnd,
    EShotActorGamestate,
    EShotActorLookat,
    EShotActorScabbardShow,
    EShotActorScabbardHide,
    EShotActorShow,
    EShotActorHide,
    EShotActorEffectStart,
    EShotActorEffectStop,
    EShotActorSound,
    EShotActorAppearance,
    EShotActorEquipRight,
    EShotActorEquipLeft,
    EShotActorUnequipRight,
    EShotActorUnequipLeft,
    EShotPropShow,
    EShotPropHide,
    EShotPropPlacement,
    EShotPropPlacementStart,
    EShotPropPlacementKey,
    EShotPropPlacementEnd,
    EShotPropEffectStart,
    EShotPropEffectStop,
    EShotEnvBlendIn,
    EShotEnvBlendOut,
    EShotFadeIn,
    EShotFadeOut,
    EShotWorldAddfact,
    EShotWorldWeather,
    EShotWorldEffectStart,
    EShotWorldEffectStop,
};

const QHash<QString, EShotActionType> StringToEShotAction = {
    { "cam",                    EShotCam                },
    { "cam.blend.start",        EShotCamBlendStart      },
    { "cam.blend.key",          EShotCamBlendKey        },
    { "cam.blend.end",          EShotCamBlendEnd        },
    { "cam.blend.togame",       EShotCamBlendTogame     },
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
    { "actor.equip.right",      EShotActorEquipRight    },
    { "actor.equip.left",       EShotActorEquipLeft     },
    { "actor.unequip.right",    EShotActorUnequipRight  },
    { "actor.unequip.left",     EShotActorUnequipLeft   },
    { "prop.show",              EShotPropShow           },
    { "prop.hide",              EShotPropHide           },
    { "prop.placement",         EShotPropPlacement      },
    { "prop.placement.start",   EShotPropPlacementStart },
    { "prop.placement.key",     EShotPropPlacementKey   },
    { "prop.placement.end",     EShotPropPlacementEnd   },
    { "prop.effect.start",      EShotPropEffectStart    },
    { "prop.effect.stop",       EShotPropEffectStop     },
    { "env.blendin",            EShotEnvBlendIn         },
    { "env.blendout",           EShotEnvBlendOut        },
    { "fade.in",                EShotFadeIn             },
    { "fade.out",               EShotFadeOut            },
    { "world.addfact",          EShotWorldAddfact       },
    { "world.weather",          EShotWorldWeather       },
    { "world.effect.start",     EShotWorldEffectStart   },
    { "world.effect.stop",      EShotWorldEffectStop    }
};
const QHash<EShotActionType, QString> EShotActionToString = {
    { EShotCam,                 "cam"                   },
    { EShotCamBlendStart,       "cam.blend.start"       },
    { EShotCamBlendKey,         "cam.blend.key"         },
    { EShotCamBlendEnd,         "cam.blend.end"         },
    { EShotCamBlendTogame,      "cam.blend.togame"      },
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
    { EShotActorEquipRight,     "actor.equip.right"     },
    { EShotActorEquipLeft,      "actor.equip.left"      },
    { EShotActorUnequipRight,   "actor.unequip.right"   },
    { EShotActorUnequipLeft,    "actor.unequip.left"    },
    { EShotPropShow,            "prop.show"             },
    { EShotPropHide,            "prop.hide"             },
    { EShotPropPlacement,       "prop.placement"        },
    { EShotPropPlacementStart,  "prop.placement.start"  },
    { EShotPropPlacementKey,    "prop.placement.key"    },
    { EShotPropPlacementEnd,    "prop.placement.end"    },
    { EShotPropEffectStart,     "prop.effect.start"     },
    { EShotPropEffectStop,      "prop.effect.stop"      },
    { EShotEnvBlendIn,          "env.blendin"           },
    { EShotEnvBlendOut,         "env.blendout"          },
    { EShotFadeIn,              "fade.in"               },
    { EShotFadeOut,             "fade.out"              },
    { EShotWorldAddfact,        "world.addfact"         },
    { EShotWorldWeather,        "world.weather"         },
    { EShotWorldEffectStart,    "world.effect.start"    },
    { EShotWorldEffectStop,     "world.effect.stop"     }
};

#endif // SHOTACTIONTYPES_H
