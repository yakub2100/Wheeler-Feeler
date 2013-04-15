


#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/TrackableResult.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/DataSet.h>

#include "SampleUtils.h"
#include "ModelLoader.h"
#include "DataStructures.h"

#ifdef __cplusplus
extern "C"
{
#endif


// Screen dimensions:
unsigned int screenWidth        = 0;
unsigned int screenHeight       = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
bool isActivityInPortraitMode   = false;

// The projection matrix used for rendering virtual objects:
QCAR::Matrix44F projectionMatrix;

// Constants:
static const float kObjectScale = 3.f;

QCAR::DataSet* wheelsDataSet= 0;

std::vector<Model*> models;
ModelLoader ml;
Model model;


int modelIndex=0;
float r=0.9,g=0.9,b=0.9;
float scale=1.5f;


JNIEXPORT void JNICALL
Java_com_yakub_wf_ModelManager_loadModels(JNIEnv *p, jobject o, jstring s)
{
	LOG("LOADING MODELS");
	const char* tmp = p->GetStringUTFChars(s,NULL);
	jsize lStringLength = p->GetStringUTFLength(s);
	char *str=(char*) malloc(sizeof(char) * (lStringLength));
	strcpy(str, tmp);

	LOG(str);
	Model *m =new Model;
	ml.loadModel(str,m);
	models.push_back(m);

	p->ReleaseStringUTFChars(s, tmp);
}

JNIEXPORT int JNICALL
Java_com_yakub_wf_GuiManager_setModel(JNIEnv *, jobject,jint i)
{
	modelIndex=i;

}

JNIEXPORT int JNICALL
Java_com_yakub_wf_GuiManager_setModelColor(JNIEnv *, jobject,jfloat red,jfloat green,jfloat blue)
{
	r=red;
	g=green;
	b=blue;

}

JNIEXPORT int JNICALL
Java_com_yakub_wf_GuiManager_scaleUp(JNIEnv *, jobject)
{
	scale=scale+0.1;
}

JNIEXPORT int JNICALL
Java_com_yakub_wf_GuiManager_scaleDown(JNIEnv *, jobject)
{
	scale=scale-0.1;
}


JNIEXPORT void JNICALL
Java_com_yakub_wf_FeelerActivity_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
{
    isActivityInPortraitMode = isPortrait;
}



JNIEXPORT int JNICALL
Java_com_yakub_wf_FeelerActivity_initTracker(JNIEnv *, jobject)
{
    LOG("Java_com_yakub_wf_FeelerActivity_initTracker");

    // Initialize the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* tracker = trackerManager.initTracker(QCAR::Tracker::IMAGE_TRACKER);
    if (tracker == NULL)
    {
        LOG("Failed to initialize ImageTracker.");
        return 0;
    }

    LOG("Successfully initialized ImageTracker.");
    return 1;
}


JNIEXPORT void JNICALL
Java_com_yakub_wf_FeelerActivity_deinitTracker(JNIEnv *, jobject)
{
    LOG("Java_com_yakub_wf_FeelerActivity_deinitTracker");

    // Deinit the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    trackerManager.deinitTracker(QCAR::Tracker::IMAGE_TRACKER);
}


JNIEXPORT int JNICALL
Java_com_yakub_wf_FeelerActivity_loadTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_yakub_wf_FeelerActivity_loadTrackerData");

    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
                    trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL)
    {
        LOG("Failed to load tracking data set because the ImageTracker has not"
            " been initialized.");
        return 0;
    }

    wheelsDataSet = imageTracker->createDataSet();
    if (wheelsDataSet == 0)
    {
        LOG("Failed to create a new tracking data.");
        return 0;
    }


    if (!wheelsDataSet->load("wheels.xml", QCAR::DataSet::STORAGE_APPRESOURCE))
    {
        LOG("Failed to load data set.");
        return 0;
    }

    // Activate the data set:
    if (!imageTracker->activateDataSet(wheelsDataSet))
    {
        LOG("Failed to activate data set.");
        return 0;
    }

    LOG("Successfully loaded and activated data set.");
    return 1;
}


JNIEXPORT int JNICALL
Java_com_yakub_wf_FeelerActivity_destroyTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_yakub_wf_FeelerActivity_destroyTrackerData");

    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
        trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL)
    {
        LOG("Failed to destroy the tracking data set because the ImageTracker has not"
            " been initialized.");
        return 0;
    }

    if (wheelsDataSet != 0)
    {
        if (imageTracker->getActiveDataSet() == wheelsDataSet &&
            !imageTracker->deactivateDataSet(wheelsDataSet))
        {
            LOG("Failed to destroy the tracking data set Tarmac because the data set "
                "could not be deactivated.");
            return 0;
        }

        if (!imageTracker->destroyDataSet(wheelsDataSet))
        {
            LOG("Failed to destroy the tracking data set Tarmac.");
            return 0;
        }

        LOG("Successfully destroyed the data set Tarmac.");
        wheelsDataSet = 0;
    }

    return 1;
}



JNIEXPORT void JNICALL
Java_com_yakub_wf_WFRenderer_renderFrame(JNIEnv *, jobject)
{

	model=*models[modelIndex];

	//LOG("Java_com_yakub_wf_GLRenderer_renderFrame");

    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_STENCIL_BUFFER_BIT);

    // Get the state from QCAR and mark the beginning of a rendering section
    QCAR::State state = QCAR::Renderer::getInstance().begin();

    // Explicitly render the Video Background
    QCAR::Renderer::getInstance().drawVideoBackground();

    // Set GL11 flags:
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);


    glEnable(GL_DEPTH_TEST);


    // Did we find any trackables this frame?
    for(int tIdx = 0; tIdx < state.getNumTrackableResults(); tIdx++)
    {
        // Get the trackable:
        const QCAR::TrackableResult* result = state.getTrackableResult(tIdx);
        const QCAR::Trackable& trackable = result->getTrackable();
        QCAR::Matrix44F modelViewMatrix = QCAR::Tool::convertPose2GLMatrix(result->getPose());


        // Load projection matrix:
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix.data);

        // Load model view matrix:
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(modelViewMatrix.data);
        glTranslatef(0.0f, 0.0f, kObjectScale);
        glScalef(scale, scale, scale);


		const GLfloat lightDiffuse[] = {1.0, 1.0, 1.0, 1.0};
		const GLfloat lightAmb[] = {0.60, 0.60, 0.60, 1.0};

		const GLfloat lightPositionSpec[] = {1.0, 1.0, 1.0, 1.0};
		const GLfloat lightSpecular[] = {1.0, 1.0, 1.0, 1.0};


		glLightfv(GL_LIGHT0, GL_POSITION, lightPositionSpec);
		glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

		//LOG("moldel %i",modelIndex);

		glEnable(GL_LIGHT0);


		for(int ii=0;ii<model.numMeshs;ii++){
			// LOG("Num meshs:%d",model.numMeshs);

			 if(strcmp("tyre",model.meshs[ii].meshName)==0 || strcmp("back",model.meshs[ii].meshName)==0){
				 //LOG("BLK");
				 //LOG(model.meshs[ii].meshName);
				 GLfloat ambientAndDiffuse[] = {0.15, 0.15, 0.15, 1.0};
				 glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);
				 GLfloat specular[] = {0.3, 0.3, 0.3, 1.0};
				 glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
				 glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0f);
			 }
			 else {
				// LOG("GREY");
				 //LOG(model.meshs[ii].meshName);
				 const GLfloat diffuse[] = {r, g, b, 1.0};
				 const GLfloat ambient[] = {r, g, b, 1.0};
				 const GLfloat specular[] = {r, g, b, 1.0};

				 glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
				 glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
				 glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
				 glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
				// GLfloat emission[] = {r, g, b, 1.0};
				// glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
			 }

			// Draw object:


			glVertexPointer(3, GL_FLOAT, sizeof(Extra), (const GLvoid*) &model.meshs[ii].extr[0].vertice);
			//LOG("Got verts");
		    glNormalPointer(GL_FLOAT, sizeof(Extra),  (const GLvoid*) &model.meshs[ii].extr[0].normal);
			//LOG("Got norms");
			glDrawArrays(GL_TRIANGLES, 0,model.meshs[ii].extr.size());
			//LOG("drawn triag");
		}
    }

    glDisable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    QCAR::Renderer::getInstance().end();
}


void
configureVideoBackground()
{
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.
                                getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


    // Configure the video background
    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;

    if (isActivityInPortraitMode)
    {
        //LOG("configureVideoBackground PORTRAIT");
        config.mSize.data[0] = videoMode.mHeight
                                * (screenHeight / (float)videoMode.mWidth);
        config.mSize.data[1] = screenHeight;

        if(config.mSize.data[0] < screenWidth)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenWidth;
            config.mSize.data[1] = screenWidth *
                              (videoMode.mWidth / (float)videoMode.mHeight);
        }
    }
    else
    {
        //LOG("configureVideoBackground LANDSCAPE");
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight
                            * (screenWidth / (float)videoMode.mWidth);

        if(config.mSize.data[1] < screenHeight)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenHeight
                                * (videoMode.mWidth / (float)videoMode.mHeight);
            config.mSize.data[1] = screenHeight;
        }
    }

    LOG("Configure Video Background : Video (%d,%d), Screen (%d,%d), mSize (%d,%d)", videoMode.mWidth, videoMode.mHeight, screenWidth, screenHeight, config.mSize.data[0], config.mSize.data[1]);

    // Set the config:
    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}


JNIEXPORT void JNICALL
Java_com_yakub_wf_FeelerActivity_initApplicationNative(
                            JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_yakub_wf_FeelerActivity_initApplicationNative");

    // Store screen dimensions
    screenWidth = width;
    screenHeight = height;

    LOG("Java_com_yakub_wf_FeelerActivity_initApplicationNative finished");
}


JNIEXPORT void JNICALL
Java_com_yakub_wf_FeelerActivity_deinitApplicationNative(
                                                        JNIEnv* env, jobject obj)
{
    LOG("Java_com_yakub_wf_FeelerActivity_deinitApplicationNative");
	//delete models from memory
	for(int i=0;i<models.size();i++){
		delete models[i];
	}

}


JNIEXPORT void JNICALL
Java_com_yakub_wf_FeelerActivity_startCamera(JNIEnv *,
                                                                         jobject)
{
    LOG("Java_com_yakub_wf_FeelerActivity_startCamera");


    // Select the camera to open, set this to QCAR::CameraDevice::CAMERA_FRONT
    // to activate the front camera instead.
    QCAR::CameraDevice::CAMERA camera = QCAR::CameraDevice::CAMERA_DEFAULT;

    // Initialize the camera:
    if (!QCAR::CameraDevice::getInstance().init(camera))
        return;

    // Configure the video background
    configureVideoBackground();

    // Select the default mode:
    if (!QCAR::CameraDevice::getInstance().selectVideoMode(
                                QCAR::CameraDevice::MODE_DEFAULT))
        return;

    // Start the camera:
    if (!QCAR::CameraDevice::getInstance().start())
        return;

    // Uncomment to enable flash
    //if(QCAR::CameraDevice::getInstance().setFlashTorchMode(true))
    //	LOG("IMAGE TARGETS : enabled torch");

    // Uncomment to enable infinity focus mode, or any other supported focus mode
    // See CameraDevice.h for supported focus modes
    //if(QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_INFINITY))
    //	LOG("IMAGE TARGETS : enabled infinity focus");

    // Start the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->start();
}


JNIEXPORT void JNICALL
Java_com_yakub_wf_FeelerActivity_stopCamera(JNIEnv *, jobject)
{
    LOG("Java_com_yakub_wf_FeelerActivity_stopCamera");

    // Stop the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->stop();

    QCAR::CameraDevice::getInstance().stop();
    QCAR::CameraDevice::getInstance().deinit();
}


JNIEXPORT void JNICALL
Java_com_yakub_wf_WFRenderer_setProjectionMatrix(JNIEnv *, jobject)
{
    LOG("Java_com_yakub_wf_FeelerActivity_setProjectionMatrix");

    // Cache the projection matrix:
    const QCAR::CameraCalibration& cameraCalibration =
                                QCAR::CameraDevice::getInstance().getCameraCalibration();
    projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f, 2500.0f);
}


JNIEXPORT jboolean JNICALL
Java_com_yakub_wf_FeelerActivity_autofocus(JNIEnv*, jobject)
{
    return QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_TRIGGERAUTO) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_yakub_wf_FeelerActivity_setFocusMode(JNIEnv*, jobject, jint mode)
{
    int qcarFocusMode;

    switch ((int)mode)
    {
        case 0:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_NORMAL;
            break;

        case 1:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO;
            break;

        case 2:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_INFINITY;
            break;

        case 3:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_MACRO;
            break;

        default:
            return JNI_FALSE;
    }

    return QCAR::CameraDevice::getInstance().setFocusMode(qcarFocusMode) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT void JNICALL
Java_com_yakub_wf_WFRenderer_initRendering(
                                                    JNIEnv* env, jobject obj)
{
    LOG("Java_com_yakub_wf_WFRenderer_initRendering");

    // Define clear color
    glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);

}


JNIEXPORT void JNICALL
Java_com_yakub_wf_WFRenderer_updateRendering(
                        JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_yakub_wf_WFRenderer_updateRendering");

    // Update screen dimensions
    screenWidth = width;
    screenHeight = height;

    // Reconfigure the video background
    configureVideoBackground();
}


#ifdef __cplusplus
}
#endif
