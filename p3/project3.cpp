//Nima Ganjehloo
//91354514
//nganjehloo@ucdavis.edu

#include <GL/glut.h>
#include <glm/glm.hpp>
#include "Math.hpp"
#include "Context.hpp"
#include "Model.hpp"
#include "Gouraud.hpp"
#include "Blinn_Phong.hpp"

#include <vector>
#include <string.h>

using namespace std;

Context ctx;

void printMat(glm::mat4 m){
	for(int i = 0; i < 4; ++i){
		for(int j = 0; j < 4; ++j){
			std::cout << m[i][j] << " ";
		}
		std::cout << endl;
	}
	std::cout << endl;
}


//set pixel in buffer
void draw_pixel(int x, int y, glm::vec3 c) {
	//use clipping bounds also as a scissor just incase
	if(x < ctx.clippt1.x || x >= ctx.clippt2.x)
		return;
	if(y < ctx.clippt1.y || y >= ctx.clippt2.y)
		return;
	//calc address of pixel and set buffer to color
	int index = y*ctx.rowsize + x*3;
	ctx.image[index + 0] = c.x;
	ctx.image[index + 1] = c.y;
	ctx.image[index + 2] = c.z;
}

//set pixel in buffer
void draw_depth(int x, int y, glm::vec3 c) {
	//use clipping bounds also as a scissor just incase
	if(x < ctx.clippt1.x || x >= ctx.clippt2.x)
		return;
	if(y < ctx.clippt1.y || y >= ctx.clippt2.y)
		return;
	//calc address of pixel and set buffer to color
	int index = y*ctx.rowsize + x*3;
	ctx.depth[index + 0] = c.x;
	ctx.depth[index + 1] = c.y;
	ctx.depth[index + 2] = c.z;
}

//generate perspective transform 
glm::mat4 perspective(glm::vec3 bmax, glm::vec3 bmin, float fov, float aspect, float n, float f){
		float t, b, l, r;
	float maxx = glm::max(fabs(bmin.x), fabs(bmax.x));
        float maxy = glm::max(fabs(bmin.y), fabs(bmax.y));
        float max = glm::max(maxx, maxy);
	float scale = glm::tan(fov * 0.5f * 3.145f / 180) * n;
	r = aspect * scale * max;
	l = -r;
	t = scale * max;
	b = -t; 
	glm::mat4 persp = glm::transpose(glm::mat4(glm::vec4((2*n)/(r-l), 0.0, (r+l)/(r-l), 0.0),
				glm::vec4(0.0, (2*n)/(t-b), (t+b)/(t-b), 0.0),
				glm::vec4(0.0, 0.0, -(f+n)/(f-n), (-2*f*n)/(f-n)),
				glm::vec4(0.0, 0.0, -1.0, 0.0)));
	return persp;
}

//generate orthographic transform
glm::mat4 orthographic(glm::vec3 bmax, glm::vec3 bmin, float aspect, float n, float f){
	float t, b, l, r;
	float maxx = glm::max(fabs(bmin.x), fabs(bmax.x));
        float maxy = glm::max(fabs(bmin.y), fabs(bmax.y));
        float max = glm::max(maxx, maxy);
	r = aspect * max;
	l = -r;
	t = max;
	b = -t; 
	
	glm::mat4 ortho = glm::transpose(glm::mat4(glm::vec4(2/(r-l), 0.0, 0.0, -(r+l)/(r-l)),
				glm::vec4(0.0, 2/(t-b), 0.0, -(t+b)/(t-b)),
				glm::vec4(0.0, 0.0, -2/(f-n), -(f+n)/(f-n)),
				glm::vec4(0.0, 0.0, 0.0, 1.0)));

	return ortho;
}

void calcViewport(int x, int y, int w, int h){
	ctx.renderstate.Viewport = glm::transpose(glm::mat4(
						glm::vec4(w/2.0f, 0, 0, x + w/2.0f),
						glm::vec4(0, -h/2.0f, 0, y + h/2.0f),
						glm::vec4(0, 0, 1, 0),
						glm::vec4(0, 0, 0, 1)));
}

void calcModelView(int cam){
	switch(cam){
	case 0:
		ctx.renderstate.ModelView = ctx.xyCam.GetViewMatrix();
		break;
	case 1:
		ctx.renderstate.ModelView = ctx.xzCam.GetViewMatrix();
		break;
	case 2:
		ctx.renderstate.ModelView = ctx.yzCam.GetViewMatrix();
		break;
	case 3:
		ctx.renderstate.ModelView = ctx.freeorthocam.GetViewMatrix();
		break;
	case 4:
		ctx.renderstate.ModelView = ctx.freeperspcam.GetViewMatrix();
		break;
	default:
		ctx.renderstate.ModelView = ctx.freeperspcam.GetViewMatrix();
		break;
	}

}

//finds min and max points of poly's and creates bounding box
void calcBounds(glm::vec3 & max, glm::vec3 & min, vector<glm::vec4> &verts){
	for(int i = 0; i < verts.size(); ++i){
			glm::vec3 vert = verts[i];
			//basic min max stuff
			if (vert.x < min.x) min.x = vert.x;
			if (vert.y < min.y) min.y = vert.y;
			if (vert.z < min.z) min.z = vert.z;
			if (vert.x > max.x) max.x = vert.x;
			if (vert.y > max.y) max.y = vert.y;
			if (vert.z > max.z) max.z = vert.z;
	}
}

void calcProjection(glm::vec3 & max, glm::vec3 & min, bool lastframe = false){
	if(ctx.viewpersp && lastframe){
		ctx.renderstate.Projection = perspective(max, min, 60.0f, ctx.xres/(float)ctx.yres, 0.1f, 1000.0f);
	}else{
		ctx.renderstate.Projection = orthographic(max, min, ctx.xres/(float)ctx.yres, 0.1f, 1000.0f);
	}
}

//bres line drawing algorithim
void draw_linebres(int x11, int x22, int y11, int y22, glm::vec3 c) {
    int x1 = x11;
    int y1 = y11;
    int x2 = x22;
    int y2 = y22;
   // delete pt;
	//set up differentials
	int dx, dy, i, error;
	int X, Y, pix1, pix2;
	int x,y;
	//setup slope vars
	dx = x2-x1;
	dy = y2-y1;
	
	//adjust slope for quadrant trick
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	X = 1;
	if (x2 < x1) X = -1;
	Y = 1;
	if (y2 < y1) Y = -1;
	x = x1; y = y1;
	
	//check slope direction to avoid severe aliasing/artifacting for steep slope
	if (dx < dy) {
		pix1 = 2*(dx-dy);
		pix2 = 2*dx;
		//error between increments on the grid/buffer
		error = 2*dx-dy;
		//utilize newtons method to solve for correct position to rasterize in current orientation
		for (i=0; i<dy; i++) {
			if (error >= 0) {
				x += X;
				error += pix1;
			}else{
				error += pix2;
			}
			y += Y;
			draw_pixel(x, y, c);
		}
	} else {
		//same as above, but reversing orientation to avoid artifacting/aliasing
		pix1 = 2*(dy-dx);
		pix2 = 2*dy;
		error = 2 * dy-dx;
		for (i=0; i<dx; i++) {
			if (error >= 0) {
				y += Y;
				error += pix1;
			}else{
				error += pix2;
			}
			x += X;
			draw_pixel(x, y, c);
		}
	}
}



//renders text
void RenderString(int x, int y, std::string str)
{  
  char *c;
   //reset opengl's internal rasterposition because of stupid way pre opengl3.0 works
   glViewport(0, 0, ctx.xres, ctx.yres-24);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, ctx.xres, ctx.yres, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
   glRasterPos2i(x,y);
  glColor3f(1.0f, 1.0f, 1.0f); 
  //render text
  for(int i = 0; i < str.length(); ++i){
	if(ctx.xres/24 > 20 && ctx.yres/24 > 10){
  		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str.c_str()[i]);
	}else{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, str.c_str()[i]);
	}
  }
}
/*
void RenderUI(){
    //render UI
    glViewport(0, 0, 0, 0);
	glRasterPos2i(0,0);
	glDrawPixels(xres, yres, GL_RGB, GL_FLOAT, image);
	//UI drawing functions
	if(Models.size() > 0){
		RenderString(0, yres/2 - 15, "XY");
		RenderString(xres/2 + 15, yres/2 - 15, "XZ");
		RenderString(0, yres/2 + 12, "YZ");
		if(viewpersp){
			RenderString(xres/2 + 12, yres/2 + 12, "Perspective");
		}else{
			RenderString(xres/2 + 12, yres/2 + 12, "Orthographic");
		}
		RenderString(0, 0, "Selected Poly: " + std::to_string(selectedpoly));
		
		//RenderString(0,24, "Rotate Angle: " + std::to_string(glm::acos(polygonlist[selectedpoly].R[0][0])));
		
		//RenderString(0,48, "Scale x " +  std::to_string(M[selectedpoly].S[0][0]) + "Scale y " + std::to_string(polygonlist[selectedpoly].S[1][1]) );
		
		//RenderString(0,72, "Trans x " + std::to_string((int)polygonlist[selectedpoly].T[2][0]) + "Trans y " + std::to_string((int)polygonlist[selectedpoly].T[2][1]));
	
		//RenderString(0,96, "Clip pt1 " + std::to_string((int)clippt1.x) + "," + std::to_string((int)clippt1.y) + "Clip pt2 " + std::to_string((int)clippt2.x) + "," +std::to_string((int)clippt2.y));
	}else{
		RenderString(0, 0, "No Poly In Scene");
	}
}*/

float edgeFunction(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2){
	return (v2.x-v0.x)*(v1.y-v0.y)-(v2.y-v0.y)*(v1.x -v0.x);
}

void rasterize(glm::vec4 v0, glm::vec4 v1, glm::vec4 v2){
	vector<glm::vec4> tverts;
	tverts.push_back(v0);
	tverts.push_back(v1);
	tverts.push_back(v2);
	glm::vec3 boundmax(-std::numeric_limits<float>::max());
    glm::vec3 boundmin(std::numeric_limits<float>::max());
	calcBounds(boundmax, boundmin, tverts);

	int x = 0;
	int y = 0;
	float area =  edgeFunction(v0, v1, v2);
	//printf("kek1");
	/*glm::vec3 c0(1.0, 0.0, 0.0);
	glm::vec3 c1(0.0, 1.0, 0.0);
	glm::vec3 c2(0.0, 0.0, 1.0);*/
	boundmin.x = glm::clamp(boundmin.x, 0.0f, (float)ctx.xres);
	boundmax.x = glm::clamp(boundmax.x, 0.0f, (float)ctx.xres);
	boundmin.y = glm::clamp(boundmin.y, 0.0f, (float)ctx.yres);
	boundmax.y = glm::clamp(boundmax.y, 0.0f, (float)ctx.yres);
	glm::vec3 color(1.0f,1.0f,1.0f);
	for(x =boundmin.x; x<boundmax.x; ++x){
		for(y =boundmin.y; y<boundmax.y; ++y){
			//printf("\nx: %d", x);
			//printf("  y: %d\n", y);
			float bx = edgeFunction(v1, v2, glm::vec3(x, y, 0));
			float by = edgeFunction(v2, v0, glm::vec3(x, y, 0));
			float bz = edgeFunction(v0, v1, glm::vec3(x, y, 0));
			
            if (bx<0 || by<0 || bz<0) continue; 
			bx /= area;
			by /= area;
			bz /= area;
			glm::vec3 bary = glm::vec3(bx, by, bz);
			float fragDepth = 1.0f/glm::dot(glm::vec3(v0.z, v1.z, v2.z), bary);
			/*float r =glm::dot(glm::vec3(c0.r, c1.r, c2.r), bary);
			float g =glm::dot(glm::vec3(c0.g, c1.g, c2.g), bary);
			float b =glm::dot(glm::vec3(c0.b, c1.b, c2.b), bary);*/

			int index = y*ctx.rowsize + x*3;
			float dx = ctx.depth[index + 0];
			float dy = ctx.depth[index + 1];
			float dz = ctx.depth[index + 2];
			//printf("fragDepth: %f\n", fragDepth);
			//printf("depth: %f, %f, %f\n", dx, dy, dx);
			if(( dx > fragDepth) && (dy > fragDepth) && (dz > fragDepth)) continue;
			//printf("bary: %f %f %f\n", bary.x, bary.y, bary.z);
			bool shade = (ctx.renderstate.boundShader)->fragment(bary, color);
			//printf("color: %f %f %f\n", color.x, color.y, color.z);
			if(!shade){
				draw_depth(x, y, glm::vec3(fragDepth, fragDepth, fragDepth));
            	draw_pixel(x, y, color);
			}
		}	
	}
}


void draw(unsigned int mdindex){
    for(int i = 0; i < ctx.models[mdindex].Loader.LoadedMeshes.size(); ++i){
		 objl::Mesh curMesh =  ctx.models[mdindex].Loader.LoadedMeshes[i];
		for(int j = 0; j < ctx.renderstate.boundIndices.size(); j+=3){
			unsigned int vi0 = ctx.renderstate.boundIndices[j];
            unsigned int vi1 = ctx.renderstate.boundIndices[j+1];
            unsigned int vi2 = ctx.renderstate.boundIndices[j+2];

			glm::vec4 v0 = ctx.renderstate.boundVerts[j];
            glm::vec4 v1 = ctx.renderstate.boundVerts[j+1];
            glm::vec4 v2 = ctx.renderstate.boundVerts[j+2];

			v0 = ctx.renderstate.boundShader->vertex(v0, mdindex, i, vi0, ctx);
			v1 = ctx.renderstate.boundShader->vertex(v1, mdindex, i, vi1, ctx);
			v2 = ctx.renderstate.boundShader->vertex(v2, mdindex, i, vi2, ctx);

			rasterize(v0, v1, v2);
		}
    }
	/*//printf("TRI 1");
	//printf("TRI 2");
	glm::vec4 v4 = renderstate.Viewport*renderstate.Projection*renderstate.ModelView*renderstate.boundVerts[3];
	glm::vec4 v5 = renderstate.Viewport*renderstate.Projection*renderstate.ModelView*renderstate.boundVerts[4];
	glm::vec4 v6 = renderstate.Viewport*renderstate.Projection*renderstate.ModelView*renderstate.boundVerts[5];
	rasterize(v4, v5, v6);*/
}

void bindVertices(std::vector<glm::vec4> & verts, std::vector<unsigned int> & indices){
	ctx.renderstate.boundVerts = verts;
	ctx.renderstate.boundIndices = indices;
}

void bindShader(IShader* & shade){
	ctx.renderstate.boundShader = shade;
}

void unbindVertices(){
	ctx.renderstate.boundVerts.clear();
	ctx.renderstate.boundIndices.clear();
}

void unbindShader(){
	delete ctx.renderstate.boundShader;
	ctx.renderstate.boundShader = nullptr;
}

void display_callback(){
	int timer = glutGet(GLUT_ELAPSED_TIME);
	ctx.deltaTime = timer-ctx.lastTime;
	glm::vec3 boundmax(-std::numeric_limits<float>::max());
    glm::vec3 boundmin(std::numeric_limits<float>::max());
	
    memset(ctx.image, 0, 3*ctx.xres*ctx.yres*sizeof(float));
	for (int i = 0; i < ctx.xres * ctx.yres * 3; ++i) 
		ctx.depth[i] = -std::numeric_limits<float>::max();
    //glClear(GL_COLOR_BUFFER_BIT);

    //DRAW GRID ON SCREEN
    //draw grid
    glm::vec3 c = {1.0, 1.0, 1.0};
    draw_linebres(ctx.xres/2, ctx.xres/2, 0, ctx.yres, c);
    draw_linebres(0, ctx.xres, ctx.yres/2, ctx.yres/2, c);

    //DO TRANSFORM XY
	//bind vertices
	{
	vector<glm::vec4> tri;
	vector<unsigned int> indices;
	ctx.models[0].setupModel(tri, indices);
	IShader * shdptr = (new Gouraud());
	bindShader(shdptr);
	bindVertices(tri, indices);
	dynamic_cast<Gouraud*>(ctx.renderstate.boundShader)->dir_light.push_back(glm::vec3(2.0, 2.0, 22.0));
	//do world transforms on bound vertices
	calcBounds(boundmax, boundmin, ctx.renderstate.boundVerts);
    //setup viewport
    calcViewport(0, ctx.yres/2, ctx.xres/2, ctx.yres/2);
	//calcBounds(boundmax, boundmin);
    calcProjection(boundmax, boundmin);
	//setup view matrix
    calcModelView(0);
    //draw bound vertices
    draw(0);
	unbindVertices();
	unbindShader();
	}

	{
    //DO TRANSFORM XZ
	vector<glm::vec4> tri;
	vector<unsigned int> indices;
	ctx.models[0].setupModel(tri, indices);
	IShader * shdptr = (new Gouraud());
	bindShader(shdptr);
	bindVertices(tri, indices);
	dynamic_cast<Gouraud*>(ctx.renderstate.boundShader)->dir_light.push_back(glm::vec3(2.0, 2.0, 22.0));
	//do world transforms on bound vertices
	calcBounds(boundmax, boundmin, ctx.renderstate.boundVerts);
    //setup viewport
    calcViewport(ctx.xres/2, ctx.yres/2, ctx.xres/2, ctx.yres/2);
	//calcBounds(boundmax, boundmin);
    calcProjection(boundmax, boundmin);
	//setup view matrix
    calcModelView(1);
    //draw bound vertices
    draw(0);
	unbindVertices();
	unbindShader();
	}

	{
    //DO TRANSFORM ZY
	vector<glm::vec4> tri;
	vector<unsigned int> indices;
	ctx.models[0].setupModel(tri, indices);
	IShader * shdptr = (new Gouraud());
	bindShader(shdptr);
	bindVertices(tri, indices);
	dynamic_cast<Gouraud*>(ctx.renderstate.boundShader)->dir_light.push_back(glm::vec3(2.0, 2.0, 22.0));
	//do world transforms on bound vertices
	calcBounds(boundmax, boundmin, ctx.renderstate.boundVerts);
    //setup viewport
    calcViewport(0, 0, ctx.xres/2, ctx.yres/2);
	//calcBounds(boundmax, boundmin);
    calcProjection(boundmax, boundmin);
	//setup view matrix
    calcModelView(2);
    //draw bound vertices
    draw(0);
	unbindVertices();
	unbindShader();
	}
	{
    //DO TRANSFORM FREE
	vector<glm::vec4> tri;
	vector<unsigned int> indices;
	ctx.models[0].setupModel(tri, indices);
	IShader * shdptr = (new Blinn_Phong());
	bindShader(shdptr);
	bindVertices(tri, indices);
	dynamic_cast<Blinn_Phong*>(ctx.renderstate.boundShader)->dir_light.push_back(glm::vec3(2.0, 2.0, 22.0));
	//do world transforms on bound vertices
	calcBounds(boundmax, boundmin, ctx.renderstate.boundVerts);
    //setup viewport
    calcViewport(ctx.xres/2, 0, ctx.xres/2, ctx.yres/2);
	//calcBounds(boundmax, boundmin);
    calcProjection(boundmax, boundmin, true);
	//setup view matrix
	if(ctx.viewpersp){
    	calcModelView(4);
		dynamic_cast<Blinn_Phong*>(ctx.renderstate.boundShader)->viewPos = ctx.renderstate.ModelView*glm::vec4(ctx.freeperspcam.Position, 1);
	}else{
		calcModelView(3);
		dynamic_cast<Blinn_Phong*>(ctx.renderstate.boundShader)->viewPos = ctx.renderstate.ModelView*glm::vec4(ctx.freeperspcam.Position, 1);
	}
    //draw bound vertices
    draw(0);
	unbindVertices();
	unbindShader();
	}

    //render scene
	if(ctx.viewdepth){
    	glDrawPixels(ctx.xres, ctx.yres, GL_RGB, GL_FLOAT, ctx.depth);
	}else{
		glDrawPixels(ctx.xres, ctx.yres, GL_RGB, GL_FLOAT, ctx.image);
	}
    //render UI
    //RenderUI();

    glFlush();
    //send event for redrawing
    glutPostRedisplay();
	ctx.lastTime = timer;
}

//dont let user reshape
void reshape_callback(int x, int y){
	glutReshapeWindow( ctx.xres, ctx.yres);
}

void
keyboard_callback(unsigned char key, int x, int y)
{
  //printf("%c", key);
  switch (key) {
  case 'c':
	//change clipping bounds for cohen-suther
	ctx.clippt1.x += 0.2f*ctx.xres;
	ctx.clippt1.y += 0.2f*ctx.yres;
	ctx.clippt2.x -= 0.2f*ctx.xres;
	ctx.clippt2.y -= 0.2f*ctx.yres;
	ctx.renderstate.clipmode = (ctx.renderstate.clipmode + 1)%4;
	if(ctx.renderstate.clipmode == 0){
		ctx.clippt1.x = 0;
		ctx.clippt1.y = 0;
		ctx.clippt2.x = ctx.xres;
		ctx.clippt2.y = ctx.yres;
	}
	break;
  case 'p':
	//change polygon that transforms apply too
	/*if(Models.size() == 0)
		break;
	if(selectedpoly +1 >= Models.size()){
		selectedpoly = 0;
		break;
	}*/
	ctx.selectedpoly++;
	break;
  case 'r':
    //rotate state
    ctx.renderstate.rotate = true;
    ctx.renderstate.scale = false;
    ctx.renderstate.translate = false;
    ctx.freecam = false;
    break;
  case 's':
    //scale state
    ctx.renderstate.rotate = false;
    ctx.renderstate.scale = true;
    ctx.renderstate.translate = false;
    ctx.freecam = false;
    break;
   case 'f':
    //rotate state
    ctx.renderstate.rotate = false;
    ctx.renderstate.scale = false;
    ctx.renderstate.translate = false;
    ctx.freecam = true;
	printf("%d", ctx.freecam);
    break;
  case 't':
    //translate state
    ctx.renderstate.rotate = false;
    ctx.renderstate.scale = false;
    ctx.renderstate.translate = true;
    ctx.freecam = false;
    break;
  case 'd':
	ctx.viewdepth = !ctx.viewdepth;
	ctx.viewpersp = ctx.viewdepth;
	break;
  case 'v':
	ctx.viewpersp = !ctx.viewpersp;
	break;
  case 27:
	//write scene to file on program exit
    exit(0);
  }
}

//allows sligh shift in camera target based on mouse position
//uses euler angles to calculate yaw and pitch which is then used to
//define target position
void mouse_move_callback(int x, int y){
    if(ctx.first)
    {
        ctx.lastX = x;
        ctx.lastY = y;
        ctx.first = false;
    }
  
    float xoffset = x - ctx.lastX;
    float yoffset = ctx.lastY - y; 
    ctx.lastX = x;
    ctx.lastY = y;

    ctx.freeorthocam.ProcessMouseMovement(xoffset, yoffset);
    ctx.freeperspcam.ProcessMouseMovement(xoffset, yoffset);
}

//transform selected poly based on stored transforms from keyboard callback
void arrow_callback(int key, int x, int y){
	switch (key) {
	  case GLUT_KEY_UP:
		//do something here
		if(ctx.freecam){
		//	printf("move");
		//	 printf("%f", ctx.deltaTime/1000.0f);
			ctx.freeorthocam.ProcessKeyboard(FORWARD, ctx.deltaTime/1000.0f);
			ctx.freeperspcam.ProcessKeyboard(FORWARD, ctx.deltaTime/1000.0f);
		}
		/*if(renderstate.translate){
			glm::mat4 tmat = polygonlist[selectedpoly].T;
			polygonlist[selectedpoly].T = tmat*glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(0.0f, 0.1f, 0.0f, 1.0f));
		}
		if(renderstate.scale){
			glm::mat4 tmat = polygonlist[selectedpoly].S;
			polygonlist[selectedpoly].S = tmat*glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 1.1f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		if(renderstate.rotate){
			glm::mat4 tmat = polygonlist[selectedpoly].R;
			glm::vec4 ax = rotaxis.R*rotaxis.linelist[0].p1;
			//cout << ax.x << ax.y << ax.z << endl;
			polygonlist[selectedpoly].R = tmat*rodriguezRot(glm::vec3(ax.x, ax.y, ax.z),10);
		}*/
		break;
	  case GLUT_KEY_DOWN:
		//do something here
		if(ctx.freecam){
			ctx.freeorthocam.ProcessKeyboard(BACKWARD, ctx.deltaTime/1000.0f);
			ctx.freeperspcam.ProcessKeyboard(BACKWARD, ctx.deltaTime/1000.0f);
		}
		/*if(renderstate.translate){
			glm::mat4 tmat = polygonlist[selectedpoly].T;
			polygonlist[selectedpoly].T = tmat*glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(0.0f, -0.1f, 0.0f, 1.0f));
		}
		if(renderstate.scale){
			glm::mat4 tmat = polygonlist[selectedpoly].S;
			polygonlist[selectedpoly].S = tmat*glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.9f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		if(renderstate.rotate){
			glm::mat4 tmat = glm::mat4(glm::vec4(glm::cos(0.1f),glm::sin(0.1f), 0.0, 0.0),
						glm::vec4( -glm::sin(0.1f), glm::cos(0.1f), 0.0, 0.0f),
						glm::vec4(0.0, 0.0f, 1.0, 0.0f),
						glm::vec4(0.0, 0.0f, 0.0f, 0.0f));
			rotaxis.R = rotaxis.R*tmat;

		}*/
		break;
	  case GLUT_KEY_LEFT:
		//do something here
		if(ctx.freecam){
			ctx.freeorthocam.ProcessKeyboard(LEFT, ctx.deltaTime/1000.0f);
			ctx.freeperspcam.ProcessKeyboard(LEFT, ctx.deltaTime/1000.0f);
		}
		/*if(renderstate.translate){
			glm::mat4 tmat = polygonlist[selectedpoly].T;
			polygonlist[selectedpoly].T = tmat*glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(-0.1f, 0.0f, 0.0f, 1.0f));
		}
		if(renderstate.scale){
			glm::mat4 tmat = polygonlist[selectedpoly].S;
			polygonlist[selectedpoly].S = tmat*glm::mat4(glm::vec4(0.9f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		if(renderstate.rotate){
			glm::mat4 tmat = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0),
								     glm::vec4(0.0, glm::cos(-0.1f), glm::sin(-0.1f), 0.0f),
								     glm::vec4( 0.0, -glm::sin(-0.1f), glm::cos(-0.1f), 0.0f),
								     glm::vec4(0.0, 0.0f, 0.0f, 0.0f));
			rotaxis.R = rotaxis.R*tmat;
		}*/
		break;
	  case GLUT_KEY_RIGHT:
		//do something here
		if(ctx.freecam){
			ctx.freeorthocam.ProcessKeyboard(RIGHT, ctx.deltaTime/1000.0f);
			ctx.freeperspcam.ProcessKeyboard(RIGHT, ctx.deltaTime/1000.0f);
		}
		/*if(renderstate.translate){
			glm::mat4 tmat = polygonlist[selectedpoly].T;
			polygonlist[selectedpoly].T = tmat*glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(0.1f, 0.0f, 0.0f, 1.0f));
		}
		if(renderstate.scale){
			glm::mat4 tmat = polygonlist[selectedpoly].S;
			polygonlist[selectedpoly].S = tmat*glm::mat4(glm::vec4(1.1f, 0.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
								     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		}
		if(renderstate.rotate){
			glm::mat4 tmat = glm::mat4(
								     glm::vec4(glm::cos(0.1f), 0.0, -glm::sin(0.1f), 0.0f),
								      glm::vec4(0.0, 1.0f, 0.0f, 0.0f),
								     glm::vec4(glm::sin(0.1f), 0.0, glm::cos(0.1f), 0.0f),
								     glm::vec4(0.0, 0.0f, 0.0f, 0.0f));
			rotaxis.R = rotaxis.R*tmat;
		}*/
		break;
	  default:
		break;
	}
}

void myInit() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glMatrixMode(GL_PROJECTION);
	//gluOrtho2D(0, 500, 0, 500);
}

//start program here
int main(int argc, char **argv) {
	//check params
	if(argc > 4 || argc < 3){
		printf("BAD PARAMS\n");
		printf("./project1 xres yres optionalscenefile\n");
		return -1;
	}
	//setup resolution & buffers
	ctx.xres = std::strtol(argv[1],0,10);
	ctx.yres = std::strtol(argv[2],0,10);
	ctx.image = new float[ 3 * ctx.xres *ctx.yres];
	ctx.depth = new float[ 3 * ctx.xres *ctx.yres];
	ctx.rowsize = 3*ctx.xres;
	//clipping bounds
	ctx.clippt2.x = ctx.xres;
	ctx.clippt2.y = ctx.yres;
	//parse scene file
	
	//setup views
	ctx.xyCam = Camera(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0,- 1.0, 0.0));
	ctx.xzCam = Camera(glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	ctx.yzCam = Camera(glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	ctx.freeorthocam = ctx.freeperspcam = Camera(glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0,- 1.0, 0.0));
	Model model(argv[3]);
	ctx.models.push_back(model);

	
	printf("starting app");
	//glut inits and call backs
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ctx.xres, ctx.yres);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Project 3");
	myInit();
	glutReshapeFunc(reshape_callback);
  	glutKeyboardFunc(keyboard_callback);
	glutSpecialFunc(arrow_callback);
	glutPassiveMotionFunc(mouse_move_callback);
	//glutMouseFunc(mouse_callback);
	glutDisplayFunc(display_callback);
	glutMainLoop();
}
