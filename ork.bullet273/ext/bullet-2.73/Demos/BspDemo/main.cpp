/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#include "BspDemo.h"
#include "GlutStuff.h"
#include "GLDebugDrawer.h"
#include "btBulletDynamicsCommon.h"

char* makeExeToBspFilename(const char* lpCmdLine);
char* getLastFileName();


int main(int argc,char** argv)
{

	BspDemo* bspDemo = new BspDemo();

	const char* bspfilename = "BspDemo.bsp";

	printf("argc=%i\n",argc);
	{
		for (int i=0;i<argc;i++)
		{
			printf("argv[%i]=%s\n",i,argv[i]);
		}
		
		bspfilename = makeExeToBspFilename(argv[0]);
		printf("new name=%s\n",bspfilename);
	}
	if (argc>1)
	{
		bspfilename = argv[1];
	}

	GLDebugDrawer	gDebugDrawer;

	// Enrico: TODO: Should change parameter type of initPhysics() to std::string or at least const char *
	bspDemo->initPhysics((char*)bspfilename);
	
	bspDemo->getDynamicsWorld()->setDebugDrawer(&gDebugDrawer);

	

	return glutmain(argc, argv,640,480,"Bullet Quake BSP Physics Viewer http://bullet.sourceforge.net",bspDemo);
}

