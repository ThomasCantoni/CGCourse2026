#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"

int width, height;



/* variables for storing the cone and cylinder  */
renderable r_quad;
bool is_dragging = false;
float pos[2];

/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (is_dragging) {
		pos[0] = (float)xpos / float(width) * 2.0 - 1.0;
		pos[1] = (height - (float)ypos) / float(height) * 2.0 - 1.0;
	}
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT)
		if ( action == GLFW_PRESS)
			is_dragging = true;
		else
			is_dragging = false;

}

/* callback function called when the mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

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
}

float alpha,R;
void gui_setup() {

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("values")) {
		ImGui::SliderFloat("isovalues", &alpha, 0.0, 1.0f);
		ImGui::SliderFloat("support radius", &R, 0.0, 1.0f);
		ImGui::EndMenu();
	}

	 ImGui::EndMainMenuBar();
}




int main(int argc, char** argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	/* Create a windowed mode window and its OpenGL context */
	width = 1000;
	height = 1000;
	window = glfwCreateWindow(width, height, "code_11_metaballs", NULL, NULL);
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

	/* initialize IMGUI */
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	/* end IMGUI initialization */

	glEnable(GL_MULTISAMPLE);

	printout_opengl_glsl_info();

	/* load the shaders */
	shader basic_shader;
	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");

	/* create a quad for the full screen quad*/
	r_quad = shape_maker::quad();
	alpha = 0.1;
	R = 0.5;

	/* set the viewport  */
	glViewport(0, 0, width, height);


	glUseProgram(basic_shader.program);
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.8f, 0.8f, 0.9f, 1.f);

		glUniform2f(basic_shader["uPos"], pos[0], pos[1]);
		glUniform1f(basic_shader["uR"], R);
		glUniform1f(basic_shader["uIso"], alpha);

		r_quad.bind();
		glDrawElements(r_quad().mode, r_quad().count, r_quad().itype, 0);

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
