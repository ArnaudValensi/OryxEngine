//---------------------------------------------------------------------------
//(C) Copyright Riley Adams 2010

//This file is part of Oryx Engine.

// Oryx Engine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Oryx Engine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with Oryx Engine. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------

#include "Ogredllmain.h"
#include "OgreSubsystem.h"
#include "OryxLogger.h"

#include "Ogre.h"
#include "OgreTextAreaOverlayElement.h"
#include "OgreFontManager.h"

#include "OryxStringUtils.h"
#include "OgreConversions.h"

namespace Oryx
{
	#define MASQUE_FULL 0xFFFFFFFF
	class StencilListener : public Ogre::RenderQueueListener
	{
	public:

		virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation) 
		{ 
			if(queueGroupId == 75)
			{
				Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 

				if(pass.first == 0)
				{
					rendersys->setStencilCheckEnabled(true); 
					rendersys->setStencilBufferParams(Ogre::CMPF_ALWAYS_PASS,
						1, MASQUE_FULL, 
						Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_REPLACE,false);       
				}
				else if(pass.first == 1)
				{
					rendersys->setStencilCheckEnabled(true); 
					rendersys->setStencilBufferParams(Ogre::CMPF_EQUAL,
						pass.second + 1, MASQUE_FULL, 
						Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_INCREMENT,false);       
				}
				/*else
				{
					rendersys->setStencilCheckEnabled(false); 
				}*/
			}
			else if(queueGroupId == 76)
			{
				Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 

				if(pass.first == 0)
				{
					rendersys->setStencilCheckEnabled(true); 
					rendersys->setStencilBufferParams(Ogre::CMPF_ALWAYS_PASS,
						51, MASQUE_FULL, 
						Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_REPLACE,false);       
				}
				else if(pass.first == 2)
				{
					rendersys->setStencilCheckEnabled(true); 
					rendersys->setStencilBufferParams(Ogre::CMPF_EQUAL,
						51 + pass.second, MASQUE_FULL, 
						Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_INCREMENT,false);       
				}
				/*else
				{
					rendersys->setStencilCheckEnabled(false); 
				}*/
			}
		} 

		virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation) 
		{ 
		} 

		std::pair<int,int> pass;
		int numRecursions;
	};

	OgreSubsystem::OgreSubsystem(unsigned int resX,unsigned int resY,bool fullscreen,
		std::map<String,String> params)
		:EngineSubsystem(),mInitialized(0),mResolutionX(resX),mResolutionY(resY),
		mFullscreen(fullscreen),mParams(params),mAutoNameIndex(0),mNeedsRestart(false){}
	//-----------------------------------------------------------------------

	OgreSubsystem::~OgreSubsystem()
	{
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::_init()
	{
		if(!mInitialized)
		{
			mPortalHack = false;
			mPortalDepth = 1;
			Logger::getPtr()->logMessage("Ogre Subsystem starting up...");

			mRoot = new Ogre::Root("","");

			mRoot->loadPlugin("OgrePlugins/RenderSystem_GL");
			mRoot->loadPlugin("OgrePlugins/Plugin_CgProgramManager");
			mRoot->loadPlugin("OgrePlugins/Plugin_ParticleFX");

			Ogre::RenderSystem* rs = mRoot->getRenderSystemByName(
				"OpenGL Rendering Subsystem");
			mRoot->setRenderSystem(rs);

			mRoot->initialise(false);

			Ogre::NameValuePairList miscP;

			miscP["vsync"] = "false";
			miscP["FSAA"] = "0";
			miscP["gamma"] = "false";
			miscP["border"] = "fixed";
			miscP["colourDepth"] = "32";
			miscP["monitorIndex"] = "0";

			mWindow = mRoot->createRenderWindow("OryxEngine Application",
				mResolutionX,mResolutionY,mFullscreen,&miscP);

			mWindow->setActive(true);

			mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC,"mSceneManager");
			Ogre::SceneNode* defaultCamNode = mSceneManager->createSceneNode("DefaultCameraNode");
			mDefaultCamera = new Camera("DefaultCamera",defaultCamNode,mSceneManager
				->createCamera("DefaultCamera"));
			defaultCamNode->attachObject(mDefaultCamera->getCamera());
			mDefaultCamera->setPosition(Vector3(0,0,0));
			mDefaultCamera->getCamera()->setDirection(0,0,-1);
			mDefaultCamera->setFarClip(60);
			mDefaultCamera->setNearClip(0.1f);
			mDefaultCamera->setFOV(60.f);

			mViewport = mWindow->addViewport(mDefaultCamera->getCamera());
			mViewport->setBackgroundColour(Ogre::ColourValue(1.0f,0.5f,0.2f));

			mDefaultCamera->getCamera()->setAspectRatio(Ogre::Real(mViewport->getActualWidth())
									/ Ogre::Real(mViewport->getActualHeight()));

			Ogre::ResourceGroupManager* resourceMgr = Ogre::ResourceGroupManager::getSingletonPtr();
			resourceMgr->addResourceLocation("../media","FileSystem","General");
			resourceMgr->addResourceLocation("../media/models","FileSystem","General");
			resourceMgr->addResourceLocation("../media/materials","FileSystem","General");
			resourceMgr->addResourceLocation("../media/textures","FileSystem","General");
			resourceMgr->addResourceLocation("../media/audio","FileSystem","General");
			resourceMgr->addResourceLocation("../media/gui","FileSystem","General");
			resourceMgr->addResourceLocation("../media/scripts","FileSystem","General");
			resourceMgr->addResourceLocation("../media/fonts","FileSystem","General");
			resourceMgr->initialiseAllResourceGroups();

			mRootSceneNode = new SceneNode(mSceneManager->getRootSceneNode()->getName(),
				mSceneManager->getRootSceneNode());

			Logger::getPtr()->logMessage("Ogre Subsystem started up.");
			mInitialized = true;
		}
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::_deinit()
	{
		if(mInitialized)
		{
			Logger::getPtr()->logMessage("Ogre Subsystem shutting down...");
			for(uint i=0;i<mSceneNodes.size();++i)
				delete mSceneNodes[i];
			mSceneNodes.clear();

			for(int i = 0; i < mScreenMeshes.size(); ++i)
				delete mScreenMeshes[i];
			mScreenMeshes.clear();

			Logger::getPtr()->logMessage("deleting Root...");
			delete mRoot;
			mRoot = 0;
			Logger::getPtr()->logMessage("Ogre Subsystem shut down.");
			mInitialized = false;
		}
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::_update(Real delta)
	{
		renderFrame();
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::_endState()
	{
		if(mNeedsRestart)
			restart();
		for(int i = 0; i < mScreenMeshes.size(); ++i)
			delete mScreenMeshes[i];
		mScreenMeshes.clear();
	}
	//-----------------------------------------------------------------------

	String OgreSubsystem::getName()
	{
		return String("OgreSubsystem");
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::renderFrame()
	{
		Ogre::WindowEventUtilities::messagePump();
		//mRoot->renderOneFrame();
		Ogre::RenderSystem * rendersys = Ogre::Root::getSingleton().getRenderSystem(); 

		if(mPortalHack)
		{

			// manually clear framebuffer
			rendersys->clearFrameBuffer(Ogre::FBT_STENCIL | Ogre::FBT_COLOUR | Ogre::FBT_DEPTH,
				mViewport->getBackgroundColour()); 

			// enable stencil, but always pass
			rendersys->setStencilCheckEnabled(true); 
			rendersys->setStencilBufferParams(Ogre::CMPF_EQUAL,
				0, MASQUE_FULL, Ogre::SOP_ZERO,Ogre::SOP_ZERO,Ogre::SOP_ZERO,false);

			// to keep track of what we're rendering: (portal index, recursion count)
			std::pair<int, int> passInfo = std::make_pair(0, 0);

			mListener->pass = passInfo;

			// render once normally (inc. portals with stencil)
			//mViewport->setMaterialScheme("test");
			getActiveCamera()->getCamera()->_notifyMoved();

			getActiveCamera()->getCamera()->_renderScene(mViewport, true);

			// from here on out we'll always need stenciling
			rendersys->setStencilCheckEnabled(true); 

			// the first portal
			passInfo.first = 1;
			passInfo.second = 0;

			for(passInfo.second = 0; passInfo.second < mPortalDepth; ++passInfo.second)
			{
				// notify
				getSignal("updateCam")->send(passInfo);
				mListener->pass = passInfo;
				getActiveCamera()->getCamera()->_notifyMoved();

				rendersys->clearFrameBuffer(Ogre::FBT_DEPTH);
				rendersys->setStencilCheckEnabled(true); 
				rendersys->setStencilBufferParams(Ogre::CMPF_EQUAL,
					passInfo.second + 1, MASQUE_FULL, Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_KEEP,false);

				//mViewport->setMaterialScheme("regular");
				getActiveCamera()->getCamera()->_renderScene(mViewport, true);
			}

			// and the second
			passInfo.first = 2;
			passInfo.second = 0;

			for(passInfo.second = 0; passInfo.second < mPortalDepth; ++passInfo.second)
			{
				// notify
				getSignal("updateCam")->send(passInfo);
				mListener->pass = passInfo;
				getActiveCamera()->getCamera()->_notifyMoved();
				//getActiveCamera()->getCamera()->updateFrustum();

				rendersys->clearFrameBuffer(Ogre::FBT_DEPTH);
				rendersys->setStencilCheckEnabled(true); 
				rendersys->setStencilBufferParams(Ogre::CMPF_EQUAL,
					51 + passInfo.second, MASQUE_FULL, Ogre::SOP_KEEP,Ogre::SOP_KEEP,Ogre::SOP_KEEP,false);

				//mViewport->setMaterialScheme("alternativo");
				//mViewport->setMaterialScheme("regular");
				getActiveCamera()->getCamera()->_renderScene(mViewport, true);
			}

			// finally swap the buffers
			mWindow->swapBuffers();

			passInfo.first = 100;
			getSignal("updateCam")->send(passInfo);
		}
		else
		{
			mRoot->renderOneFrame();
			//mWindow->update(true);
		}
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::restart()
	{
		// save some stuff
		Ogre::Camera* cam = mViewport->getCamera();
		Ogre::ColourValue back = mViewport->getBackgroundColour();

		// get rid of the original window...
		mWindow->removeAllViewports();
		mRoot->detachRenderTarget(mWindow->getName());
		mWindow->destroy();

		// and make a new one
		Ogre::NameValuePairList miscP;
		miscP["vsync"] = "false";
		miscP["FSAA"] = "0";
		miscP["gamma"] = "false";
		miscP["border"] = "fixed";
		miscP["colourDepth"] = "32";
		miscP["monitorIndex"] = "0";
		mWindow = mRoot->createRenderWindow("OryxEngine Application",
			mResolutionX,mResolutionY,mFullscreen,&miscP);
		mWindow->setActive(true);

		mViewport = mWindow->addViewport(cam);
		mViewport->setBackgroundColour(back);
		cam->setAspectRatio(Ogre::Real(mViewport->getActualWidth())
			/ Ogre::Real(mViewport->getActualHeight()));

		mNeedsRestart = false;
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setBackgroundColor(Colour bc)
	{
		mViewport->setBackgroundColour(convertOgre(bc));
	}
	//-----------------------------------------------------------------------

	Colour OgreSubsystem::getBackgroundColor()
	{
		return convertOgre(mViewport->getBackgroundColour());
	}
	//-----------------------------------------------------------------------

	Ogre::Viewport* OgreSubsystem::getMainViewport()
	{
		return mViewport;
	}
	//------------------------------------------------------------------------

	SceneNode* OgreSubsystem::createSceneNode(String name)
	{
		String nombre = name;
		if(name=="AUTO_NAME_ME")
		{
			nombre = "OryxSceneNodeAutoNamed"+StringUtils::toString(mAutoNameIndex);
			++mAutoNameIndex;
		}
		mSceneNodes.push_back(new SceneNode(nombre,mSceneManager->createSceneNode(nombre)));
		return mSceneNodes.back();
	}
	//-----------------------------------------------------------------------

	Mesh* OgreSubsystem::createMesh(String mesh,String name)
	{
		String nombre = name;
		if(name=="AUTO_NAME_ME")
		{
			nombre = "OryxSceneNodeAutoNamed"+StringUtils::toString(mAutoNameIndex);
			++mAutoNameIndex;
		}
		Ogre::Entity* ent = mSceneManager->createEntity(nombre,mesh);
		Ogre::SceneNode* node  = mSceneManager->createSceneNode(nombre);
		node->attachObject(ent);
		ent->setCastShadows(false);
		Mesh* m = new Mesh(nombre,node,ent);
		mSceneNodes.push_back(m);
		return m;
	}
	//-----------------------------------------------------------------------

	Mesh* OgreSubsystem::createMesh(const MeshData& data,String name)
	{
		 String nombre = name;
		if(name=="AUTO_NAME_ME")
		{
			nombre = "OryxSceneNodeAutoNamed"+StringUtils::toString(mAutoNameIndex);
			++mAutoNameIndex;
		}

		using namespace Ogre;

		bool hasVertexColor = data.getDiffuse();
		bool hasNormals = data.getNormals();

		int numFaces = data.indices.size()/3;
		int numVertices = data.vertices.size()/3;

		HardwareVertexBufferSharedPtr posVertexBuffer;
		HardwareVertexBufferSharedPtr normVertexBuffer;
		std::vector<HardwareVertexBufferSharedPtr> texcoordsVertexBuffer;
		HardwareVertexBufferSharedPtr diffuseVertexBuffer;
		HardwareIndexBufferSharedPtr indexBuffer;

		Ogre::Mesh* m = Ogre::MeshManager::getSingletonPtr()->createManual(
		nombre,ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get();

		Ogre::SubMesh* sm = m->createSubMesh();
		sm->useSharedVertices = false;
		sm->vertexData = new VertexData();
		sm->vertexData->vertexStart = 0;
		sm->vertexData->vertexCount = numVertices;

		Ogre::VertexDeclaration* vdecl = sm->vertexData->vertexDeclaration;
		Ogre::VertexBufferBinding* vbind = sm->vertexData->vertexBufferBinding;

		size_t bufferCount = 0;

		vdecl->addElement(bufferCount, 0, VET_FLOAT3, VES_POSITION);

		if(hasNormals)
			vdecl->addElement(++bufferCount, 0, VET_FLOAT3, VES_NORMAL);

		if(hasVertexColor)
			vdecl->addElement(++bufferCount, 0, VET_FLOAT4, VES_DIFFUSE);

		for(int i=0;i<data.texcoords.size();++i)
			vdecl->addElement(++bufferCount, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES,i);

		bufferCount = 0;

		// Positions
		posVertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
			3*sizeof(float),numVertices,Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

		vbind->setBinding(bufferCount, posVertexBuffer);

		float* vertices = data.getVertices();
		float* normals = data.getNormals();
		float* diffuse = data.getDiffuse();
		unsigned short* indices = data.getIndices();

		posVertexBuffer->writeData(0,posVertexBuffer->getSizeInBytes(),vertices, true);

		// Normals
		if(hasNormals)
		{
			normVertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
			3*sizeof(float),numVertices,HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

			vbind->setBinding(++bufferCount, normVertexBuffer);

			normVertexBuffer->writeData(0,normVertexBuffer->getSizeInBytes(),normals, true);
		}

		if(hasVertexColor)
		{
			diffuseVertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
			4*sizeof(float),numVertices,HardwareBuffer::HBU_STATIC_WRITE_ONLY);

			vbind->setBinding(++bufferCount, diffuseVertexBuffer);

			diffuseVertexBuffer->writeData(0,diffuseVertexBuffer->getSizeInBytes(), diffuse, true);
		}

		// Texcoords
		for(int i=0;i<data.texcoords.size();++i)
		{
			texcoordsVertexBuffer.push_back(HardwareBufferManager::getSingleton().createVertexBuffer(
			2*sizeof(float),numVertices,HardwareBuffer::HBU_STATIC_WRITE_ONLY));

			vbind->setBinding(++bufferCount, texcoordsVertexBuffer[i]);

			texcoordsVertexBuffer[i]->writeData(0,sizeof(float)*data.texcoords[i].size(),&data.texcoords[i][0], false);
		}

		if(!data.indices.empty())
		{
			// Prepare buffer for indices
			indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT,3*numFaces,HardwareBuffer::HBU_STATIC_WRITE_ONLY, true);

			//unsigned short *faceVertexIndices = (unsigned short*)
			//indexBuffer->lock(0, numFaces*3*2, HardwareBuffer::HBL_DISCARD);

			// Set index buffer for this submesh
			sm->indexData->indexBuffer = indexBuffer;
			sm->indexData->indexStart = 0;
			sm->indexData->indexCount = 3*numFaces;

			indexBuffer->writeData(0,indexBuffer->getSizeInBytes(),indices,true);
		}

		//vdecl->sort();

		m->load();
		m->touch();

		//int dims = 16;
		//m->_setBounds(AxisAlignedBox(-dims/2,-64/2,-dims/2,dims/2,64/2,dims/2));
		m->_setBounds(AxisAlignedBox(-8.f,-64.f,-8.f,8.f,64.f,8.f));
		m->_setBoundingSphereRadius(sqrt(16*16*2)/2);//11.313f);

		sm->setMaterialName("MeinKraft");

		Ogre::Entity* ent = mSceneManager->createEntity(nombre,m->getName());
		Ogre::SceneNode* node = mSceneManager->createSceneNode(nombre);
		node->attachObject(ent);
		ent->setCastShadows(true);
		Mesh* mm = new Mesh(nombre,node,ent);
		mSceneNodes.push_back(mm);
		return mm;
	}
	//-----------------------------------------------------------------------
	
	Camera* OgreSubsystem::createCamera(String name)
	{
		String nombre = name;
		if(name=="AUTO_NAME_ME")
		{
			nombre = "OryxSceneNodeAutoNamed"+StringUtils::toString(mAutoNameIndex);
			++mAutoNameIndex;
		}
		Ogre::Camera* cam = mSceneManager->createCamera(nombre);
		Ogre::SceneNode* node  = mSceneManager->createSceneNode(nombre);
		node->attachObject(cam);
		Camera* c = new Camera(nombre,node,cam);
		c->setFarClip(60);
		c->setNearClip(0.1f);
		c->setFOV(70.f);
		mSceneNodes.push_back(c);
		return c;
	}
	//-----------------------------------------------------------------------
	Light* OgreSubsystem::createLight(String name)
	{
		String nombre = name;
		if(name=="AUTO_NAME_ME")
		{
			nombre = "OryxSceneNodeAutoNamed"+StringUtils::toString(mAutoNameIndex);
			++mAutoNameIndex;
		}
		Ogre::Light* light = mSceneManager->createLight(nombre);
		light->setCastShadows(true);
		light->setAttenuation(500,0,0,0);
		Ogre::SceneNode* node  = mSceneManager->createSceneNode(nombre);
		node->attachObject(light);
		Light* l = new Light(nombre,node,light);
		mSceneNodes.push_back(l);
		return l;
	}
	//-----------------------------------------------------------------------

	ParticleSystem* OgreSubsystem::createParticleSystem(String script,String name)
	{
		String nombre = name;
		if(name=="AUTO_NAME_ME")
		{
			nombre = "OryxSceneNodeAutoNamed"+StringUtils::toString(mAutoNameIndex);
			++mAutoNameIndex;
		}
		Ogre::ParticleSystem* psys = mSceneManager->createParticleSystem(nombre,script);
		Ogre::SceneNode* node  = mSceneManager->createSceneNode(nombre);
		node->attachObject(psys);
		//psys->setSpeedFactor(0.3f);
		ParticleSystem* ps = new ParticleSystem(nombre,node,psys);
		mSceneNodes.push_back(ps);
		return ps;
	}
	//-----------------------------------------------------------------------
	
	void OgreSubsystem::destroySceneNode(SceneNode* node)
	{
		for(int i=0;i<mSceneNodes.size();++i)
		{
			if(node==mSceneNodes[i])
			{
				delete node;
				mSceneNodes.erase(mSceneNodes.begin()+i);
				Ogre::MeshManager::getSingleton().unloadUnreferencedResources();
				break;
			}
		}
	}
	//-----------------------------------------------------------------------
	
	SceneNode* OgreSubsystem::getRootSceneNode()
	{
		return mRootSceneNode;
	}
	//--------------------------------------------------------------------------

	Camera* OgreSubsystem::getDefaultCamera()
	{
		return mDefaultCamera;
	}
	//--------------------------------------------------------------------------

	Camera* OgreSubsystem::getActiveCamera()
	{
		return mActiveCamera;
	}
	//--------------------------------------------------------------------------

	void OgreSubsystem::setActiveCamera(Camera* cam)
	{
		mActiveCamera = cam;
		mViewport->setCamera(cam->getCamera());
	}
	//--------------------------------------------------------------------------

	void OgreSubsystem::setLinearFog(float start, float end, Colour color)
	{
		mSceneManager->setFog(Ogre::FOG_LINEAR,convertOgre(color),0.f,start,end);
	}
	//-----------------------------------------------------------------------
	
	void OgreSubsystem::takeScreenshot(String name, String target)
	{
		if(target=="NULL")
			mWindow->writeContentsToFile(name + ".png");
		else
			mRoot->getRenderSystem()->getRenderTarget(target)->writeContentsToFile(name + ".png");
	}

	unsigned int OgreSubsystem::getScreenWidth()
	{
		if(mWindow)
			return mWindow->getWidth();
		return 0;
	}
	//-----------------------------------------------------------------------

	unsigned int OgreSubsystem::getScreenHeight()
	{
		if(mWindow)
			return mWindow->getHeight();
		return 0;
	}
	//-----------------------------------------------------------------------

	bool OgreSubsystem::isFullscreen()
	{
		return mWindow&&mWindow->isFullScreen();
	}
	//-----------------------------------------------------------------------

	String OgreSubsystem::getGfxParameter(String param)
	{
		if(mCurrentParams.find(param)!=mCurrentParams.end())
			return mCurrentParams[param];
		return "";
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setScreenWidth(unsigned int resX)
	{
		if(resX>0)
		{
			mResolutionX = resX;
			if(resX!=getScreenWidth())
				mNeedsRestart=true;
		}
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setScreenHeight(unsigned int resY)
	{
		if(resY>0)
		{
			mResolutionY = resY;
			if(resY!=getScreenHeight())
				mNeedsRestart=true;
		}
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setFullscreen(bool fs)
	{
		mFullscreen = fs;
		if(fs!=isFullscreen())
			mNeedsRestart = true;
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setGfxParameter(String param,String value)
	{
		if(mParams.find(param)!=mParams.end())
		{
			mParams[param] = value;
			if(getGfxParameter(param)!=value)
				mNeedsRestart = true;
		}
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::add3dOverlay(String overlay, SceneNode* node)
	{
		Ogre::OverlayManager::getSingletonPtr()->getByName(overlay)->add3D(node->getNode());
	}
	//-----------------------------------------------------------------------
	
	size_t OgreSubsystem::getWindowHandle()
	{
		size_t handle = 0;
		mWindow->getCustomAttribute("WINDOW", &handle);
		return handle;
	}
	//-----------------------------------------------------------------------

	ScreenMesh* OgreSubsystem::createScreenMesh(String material)
	{
		mScreenMeshes.push_back(new ScreenMesh(material));
		return mScreenMeshes.back();
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setOverlayEnabled(String name, bool enabled)
	{
		if(enabled)
			Ogre::OverlayManager::getSingletonPtr()->getByName(name)->show();
		else
			Ogre::OverlayManager::getSingletonPtr()->getByName(name)->hide();
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setOverlayText(String element, String text)
	{
		Ogre::OverlayManager::getSingletonPtr()->getOverlayElement(element)->setCaption(text);
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setOverlayTextColour(String element, Colour color)
	{
		Ogre::OverlayManager::getSingletonPtr()->getOverlayElement(element)->setColour(convertOgre(color));
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::setOverlayTextColour(String element, Colour color, Colour color2)
	{
		Ogre::TextAreaOverlayElement* elem = static_cast<Ogre::TextAreaOverlayElement*>(
			Ogre::OverlayManager::getSingletonPtr()->getOverlayElement(element));
		elem->setColourBottom(convertOgre(color));
		elem->setColourTop(convertOgre(color2));
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::loadFont(String name)
	{
		Ogre::FontManager::getSingletonPtr()->load(name, "General");
	}
	//-----------------------------------------------------------------------

	int OgreSubsystem::getBatchCount()
	{
		return mWindow->getStatistics().batchCount;
	}
	//-----------------------------------------------------------------------

	void OgreSubsystem::enablePortalHack(int depth)
	{
		mPortalHack = true;
		mPortalDepth = depth;
		mListener = new StencilListener;
		mListener->pass = std::make_pair(0,0);
		mListener->numRecursions = depth;
		mSceneManager->addRenderQueueListener(mListener);
		mViewport->setClearEveryFrame(false);
		createSignal("updateCam");
	}
}
