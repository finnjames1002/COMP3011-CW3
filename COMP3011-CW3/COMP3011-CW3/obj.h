#pragma once

class Obj
{
public:
	virtual void ModelTree(glm::mat4* model) = 0;
	virtual void ModelSun(glm::mat4* model) = 0;
	virtual void ModelFloor(glm::mat4* model) = 0;
	virtual void ModelSheep(glm::mat4* model) = 0;
	virtual vector<Object> ParseTree() = 0;
	virtual vector<Object> ParseFloor() = 0;
	virtual vector<Object> ParseSheep() = 0;
};

class Obj1 : public Obj
{
public:
	void ModelTree(glm::mat4* model)
	{
		//(*model) = glm::translate((*model), glm::vec3(0.f, 0.f, 0.f));
		//(*model) = glm::rotate((*model), (float)glfwGetTime() / 2, glm::vec3(0.f, 1.f, 0.f));
		//(*model) = glm::scale((*model), glm::vec3(.5f, .5f, .5f));
	}
	void ModelSun(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(0.f, 20.f, 60.f));
		(*model) = glm::rotate((*model), glm::radians(180.0f), glm::vec3(1.f, 0.f, 0.f)); // Rotate 90 degrees around X-axis
		(*model) = glm::scale((*model), glm::vec3(2.f, 2.f, 2.f));
	}
	void ModelFloor(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(0.f, 0.0f, 0.f));
		(*model) = glm::rotate((*model), glm::radians(90.0f), glm::vec3(1.f, 0.f, 0.f)); // Rotate 90 degrees around X-axis
		(*model) = glm::scale((*model), glm::vec3(200.f, 200.f, 200.f));
		//(*model) = glm::scale((*model), glm::vec3(10.f, 10.f, 10.f));
	}
	void ModelSheep(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(5.f, 0.f, 5.f));
		
	}
	void ModelLamp(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(-3.f, 0.f, 1.f));

	}
	virtual vector<Object> ParseTree()
	{
		vector<Object> objs;
		obj_parse("objs/tree/Tree.obj", &objs);
		return objs;
	}
	virtual vector<Object> ParseFloor()
	{
		vector<Object> objs;
		obj_parse("objs/floor/grass.obj", &objs);
		return objs;
	}
	virtual vector<Object> ParseSheep()
	{
		vector<Object> objs;
		obj_parse("objs/sheep/sheep.obj", &objs);
		return objs;
	}
	virtual vector<Object> ParseLamp()
	{
		vector<Object> objs;
		obj_parse("objs/streetlamp/StreetLamp.obj", &objs);
		return objs;
	}
};
