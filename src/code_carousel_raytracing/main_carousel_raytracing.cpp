#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "./3dparty/nanosvg/src/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "./3dparty/nanosvg/src/nanosvgrast.h"


#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"



#include "../common/matrix_stack.h"
#include "../common/intersection.h"
#include "../common/trackball.h"




/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "../common/gltf_loader.h"
#include "../common/texture.h"

#include "..\common\carousel\carousel.h"
#include "..\common\carousel\carousel_to_renderable.h"
#include "..\common\carousel\carousel_loader.h"


int width, height;

/* light direction in world space*/
glm::vec4 Ldir;

/* trackball to manipulate the scene */
trackball tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

/* variables for storing the cone and cylinder  */
renderable  r_quad;

/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	tb.mouse_move(proj, view, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		tb.mouse_press(proj, view, xpos, ypos);
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			tb.mouse_release();
		}
}

/* callback function called when the mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	tb.mouse_scroll(xoffset, yoffset);
}

/* callback function called when a key is pressed */
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}

/* callback function called when the windows is resized */
void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);
}



bool draw_i[2] = {true,false};
/* menu bar definition */
void gui_setup() {

	ImGui::BeginMainMenuBar();

	ImGui::EndMainMenuBar();
}



int main(int argc , char ** argv)
{
	race r;
	carousel_loader::load("small_test.svg", "terrain_256.png", r);

	//add 10 cars
	for (int i = 0; i < 10; ++i)
		r.add_car();


	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	/* Create a windowed mode window and its OpenGL context */
	width = 1024;
	height = 1024;
	window = glfwCreateWindow(width, height, "code_carousel_raytracing", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* declare the callback functions on mouse events */
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Load GL symbols *after* the context is current
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::fprintf(stderr, "Failed to initialize GLAD\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return EXIT_FAILURE;
	}
	check_gl_errors(__LINE__, __FILE__);

	
	// Read the maximum number of thread per workgroup on every dimension
	GLuint maxWorkGroupSize[3];
	GLuint maxWorkGroupCount[3];
	GLint maxWorkGroupInvocations;
	check_gl_errors(__LINE__, __FILE__);
	int workGroupSizes[3] = { 0 };
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSizes[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSizes[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSizes[2]);
	check_gl_errors(__LINE__, __FILE__);

	std::cout << "Max Workgroup Count (X, Y, Z): "
		<< workGroupSizes[0] << ", "
		<< workGroupSizes[1] << ", "
		<< workGroupSizes[2] << std::endl;
	check_gl_errors(__LINE__, __FILE__);

	// read the maximum number of invokations per workgroup 
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxWorkGroupInvocations);
	std::cout << "Max Workgroup Invocations: " << maxWorkGroupInvocations << std::endl;

	check_gl_errors(__LINE__, __FILE__);

	/* initialize IMGUI */
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	/* end IMGUI initialization */

	printout_opengl_glsl_info();

	/* create a quad for full screen wuad*/
	r_quad = shape_maker::quad();

	check_gl_errors(__LINE__, __FILE__);
	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(45.f), 1.f, 1.f, 1000.f);
	view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));


	glm::mat4 proj_inv = glm::inverse(proj);
	glm::mat4 view_inv = glm::inverse(view);

	/* Light direction is initialized as +Y */
	Ldir = glm::vec4(0, 1, 0,0);

	 

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	matrix_stack stack;

	// COMPUTE SHADERS STUFF

	///* create a   shader */

	shader texture_shader;
	texture_shader.create_program("./shaders/texture.vert", "./shaders/texture.frag");





	shader raytracer;
	raytracer.create_program(join("./shaders/directives.glsl", "./shaders/phong_material.glsl", "./shaders/phong.glsl","./shaders/cs_raytracer.comp"));
	
	check_gl_errors(__LINE__, __FILE__);
	// pass the data to the compute shader

	 // THE TERRAIN
	glUseProgram(raytracer.program);
	glm::vec4 terrain_rect_xz = r.ter().rect_xz;
	glUniform4fv(raytracer["terrain_rect"], 1, &terrain_rect_xz[0]);

	// pass a Phong material
	float a_color[3] = { 0.45f,0.15f,0.15f };
	float d_color[3] = { 0.5f,0.1f,0.2f };
	float s_color[3] = { 0.0f,0.0f,0.0f };
	float e_color[3] = { 0.5f,0.1f,0.2f };
	float l_color[3] = { 0.9f,0.9f,0.9f };
	float shininess = 1.0;
	glUniform3fv(raytracer["uDiffuseColor"], 1, &d_color[0]);
	glUniform3fv(raytracer["uAmbientColor"], 1, &a_color[0]);
	glUniform3fv(raytracer["uSpecularColor"], 1, &s_color[0]);
	glUniform1f(raytracer["uShininess"], shininess);
	glUniform3fv(raytracer["uLightColor"], 1, &l_color[0]);



	// THE TREES
	std::vector<stick_object> trees = r.trees();

	GLuint ssbo_trees;
	glGenBuffers(1, &ssbo_trees);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_trees);

	// Allochiamo e carichiamo i dati
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		trees.size() * sizeof(stick_object),
		&trees[0],
		GL_STATIC_DRAW);

	// Bind al binding point 0
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_trees);

	glUniform1i(raytracer["n_trees"],trees.size());

	// THE LAMPS
	std::vector<stick_object> lamps = r.lamps();

	GLuint ssbo_lamps;
	glGenBuffers(1, &ssbo_lamps);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_lamps);

	// Allochiamo e carichiamo i dati
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		lamps.size() * sizeof(stick_object),
		&lamps[0],
		GL_STATIC_DRAW);

	// Bind al binding point 0
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_lamps);

	glUniform1i(raytracer["n_lamps"], r.lamps().size());
	glUniform1i(raytracer["n_cars"], r.cars().size());
	glUniform1i(raytracer["n_cameramen"], r.cameramen().size());

	check_gl_errors(__LINE__, __FILE__);
		
	GLuint ssbo_cars;
	glGenBuffers(1, &ssbo_cars);

	GLuint ssbo_cameramen;
	glGenBuffers(1, &ssbo_cameramen);

	check_gl_errors(__LINE__, __FILE__);

	// create a texture    
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);

	/* this establishes that this texture is bound to the "image unit" 0.
	*  If a compute shader read/write to  the image unit 0, it is reading/writing
	* to this texture
	*/
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glUseProgram(texture_shader.program);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(texture_shader["uColorImage"], 10);
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);


	/* set the viewport  */
	glViewport(0, 0, width, height);


	/* set the trackballs position */
	tb.set_center_radius(glm::vec3(0, 0, 0), 1.f);

	
	glActiveTexture(GL_TEXTURE0);
	

	glEnable(GL_DEPTH_TEST);


	r.start(11, 0, 0, 600);
	r.update();

/* Loop until the user closes the window */
	int nf = 0;
	int cstart = clock();
	while (!glfwWindowShouldClose(window))
	{
		if (clock() - cstart > CLOCKS_PER_SEC) {
			std::cout << nf << std::endl;
			nf = 0;
			cstart = clock();
		}
		nf++;
		r.update();
		glUseProgram(raytracer.program);

		// CARS
		std::vector<glm::mat4> frames;
		for (unsigned int i = 0; i < r.cars().size(); ++i)
			frames.push_back(r.cars()[i].frame);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cars);

		glBufferData(GL_SHADER_STORAGE_BUFFER,
			frames.size() * sizeof(glm::mat4),
			&frames[0],
			GL_STATIC_DRAW);

		// Bind al binding point 1
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2, ssbo_cars);


		// CAMERAMEN
		std::vector<glm::mat4> frames_cameramen;
		for (unsigned int i = 0; i < r.cameramen().size(); ++i)
			frames_cameramen.push_back(r.cameramen()[i].frame);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cameramen);

		glBufferData(GL_SHADER_STORAGE_BUFFER,
			frames_cameramen.size() * sizeof(glm::mat4),
			&frames_cameramen[0],
			GL_STATIC_DRAW);

		// Bind al binding point 1
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_cameramen);


		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		stack.load_identity();

		stack.push();
		stack.mult(tb.matrix());

		// scaling
		float s = 1.f / r.bbox().diagonal();
		glm::vec3 c = r.bbox().center();

		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
		stack.mult(glm::translate(glm::mat4(1.f), -c));

		 

		glUniformMatrix4fv(raytracer["uProjInv"], 1, GL_FALSE, &proj_inv[0][0]);
		glUniformMatrix4fv(raytracer["uViewInv"], 1, GL_FALSE, &view_inv[0][0]);
		glm::mat4 stackInv = glm::inverse(stack.m());
		glUniformMatrix4fv(raytracer["uModelInv"], 1, GL_FALSE, &stackInv[0][0]);


		glDispatchCompute(1024   , 1024  , 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


		/* draw  a FSQ with the texture stitched on it, to show
		* the result of the compute shader
		*/
		glUseProgram(texture_shader.program);
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		r_quad.bind();
		glDrawElements(r_quad().mode, r_quad().count, r_quad().itype, 0);
 
		stack.pop();

		/* draw the Graphical User Interface */
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
		gui_setup();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		/* end of graphical user interface */


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glUseProgram(0);
	glfwTerminate();
	return 0;
}
