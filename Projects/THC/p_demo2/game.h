#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// 自定义
#include <custom/InputController.h>
#include <custom/Camera.h>
#include <custom/window.h>
#include <custom/Projection.h> 
#include <custom/ModelTrans.h> 


#include <custom/shader_m.h>
#include <custom/renderer.h>

#include "custom/model_anim_data.h"
#include "custom/animation.h"
#include "custom/animator.h"
#include <custom/model_base.h>
#include "custom/model_animated.h"
#include <custom/model_static.h>
#include <games/object.h>
#include <games/character.h>
#include <games/enemy.h>
#include <games/CombatSystem.h>
#include <custom/timer.h>
#include <games/PlayController.h>
#include <UI/iui.h>

class Game {
public:
	Game() {};
	static void init();
};