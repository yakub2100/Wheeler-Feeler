package com.yakub.wf;



import com.qualcomm.QCAR.QCAR;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.ViewGroup.LayoutParams;
import android.widget.RelativeLayout;
import android.app.Activity;
import android.content.pm.ActivityInfo;


/**
 * Main activity class
 * @author yakub
 * QCAR's ImageTargets.java was used as an example to create this class
 */
public class FeelerActivity extends Activity{
	
	 /**
     * Loads native libraries
     */
    static{
        loadLibrary("QCAR");
        loadLibrary("feeler");
    }
	
	private ModelManager modelManager;
	private GuiManager guiManager;
    private GLView glView;
    private WFRenderer renderer;
    
	private int screenWidth,screenHeight;
	private boolean cameraOn=false;
	
	// Focus mode constants:
    private static final int FOCUS_MODE_NORMAL = 0;
    private static final int FOCUS_MODE_CONTINUOUS_AUTO = 1;
    
    /** Tells native code whether we are in portait or landscape mode */
    private native void setActivityPortraitMode(boolean isPortrait);
    
    /** Native focus functions**/
    private native boolean autofocus();
    private native boolean setFocusMode(int mode);
	
	/** Native tracker initialization and deinitialization. */
    public native int initTracker();
    public native void deinitTracker();
    
    /** Native function to deinitialize the application.*/
    private native void deinitApplicationNative();
    
    /** Native function to initialize the application. */
    private native void initApplicationNative(int width, int height);

    /** Native functions to load and destroy tracking data. */
    public native int loadTrackerData();
    public native void destroyTrackerData();
    
    /** Native methods for starting and stopping the camera. */
    private native void startCamera();
    private native void stopCamera();

	@Override
	protected void onCreate(Bundle savedInstanceState){	
		super.onCreate(savedInstanceState);
		
		
		//lock to landscape screen orientation
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        
    	//load models
		modelManager=new ModelManager();
		modelManager.loadModels(getApplicationContext());
		
		//Initialize necessary things
		initApp();
		initQCAR();
		initTracker();
		initAppAR();
		
		if(loadTrackerData()==0){
			Log.e("WF","Failed to load tracker data");
			// Exiting application:
            System.exit(1);
		}
		
		//activate renderer
		renderer.isActive=true;
		
		//add GL view
		addContentView(glView, new LayoutParams(LayoutParams.MATCH_PARENT,
                 LayoutParams.MATCH_PARENT));
         
        //add GUI overlay view
        addContentView(guiManager.getGuiView(),
                 new LayoutParams(
                 LayoutParams.MATCH_PARENT,
                 LayoutParams.MATCH_PARENT));
        
        guiManager.setupGui();
		
        startCamera();
        cameraOn=true;
        if (!setFocusMode(FOCUS_MODE_CONTINUOUS_AUTO)){
            setFocusMode(FOCUS_MODE_NORMAL);
        }		
	}
	
	/** Called when the activity will start interacting with the user.*/
    protected void onResume(){
        super.onResume();

        // QCAR-specific resume operation
        QCAR.onResume();

        //camera can only be turned on when it is off
        if (!cameraOn){
            startCamera();
            cameraOn=true;
        }

        // Resume the GL view:
        if (glView != null){
            glView.setVisibility(View.VISIBLE);
            glView.onResume();
        }
        
        if (guiManager != null){
            guiManager.setupGui();
        }
    }
    
    /** Called when the system is about to start resuming a previous activity.*/
    protected void onPause(){
        super.onPause();

        if (glView != null){
        	glView.setVisibility(View.INVISIBLE);
            glView.onPause();
        }

        if (cameraOn){
            stopCamera();
            cameraOn=false;
        }

        // QCAR-specific pause operation
        QCAR.onPause();
    }
	
    /** The final call you receive before your activity is destroyed.*/
    protected void onDestroy()
    {
        super.onDestroy();

        // Do application deinitialization in native code:
        deinitApplicationNative();

        // Destroy the tracking data set:
        destroyTrackerData();

        // Deinit the tracker:
        deinitTracker();

        // Deinitialize QCAR SDK:
        QCAR.deinit();

        System.gc();
    }
    
    
	/**
	 * Init non AR related elements
	 */
	private void initApp(){
		
		// Query display dimensions:
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        screenWidth = metrics.widthPixels;
        screenHeight = metrics.heightPixels;
        
        
        setActivityPortraitMode(false);

        // As long as this window is visible to the user, keep the device's
        // screen turned on and bright:
        getWindow().setFlags(
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}
	
	/**
	 * Initializes QCAR
	 */
	private void initQCAR(){
		
		//set QCAR to use OpenGL ES 1.1
		QCAR.setInitParameters(FeelerActivity.this, QCAR.GL_11);
		int i=0;
		//initialize QCAR
		while(i < 100){
			i=QCAR.init();
			if(i<0){
				Log.e("WF", "Error initilising QCAR" );
				// Exiting application:
                System.exit(1);
			}
		}
	}
	
	 /** 
	  * Initializes AR application components. 
	  **/
    private void initAppAR()
    {
        // Do application initialization in native code (e.g. registering
        // callbacks, etc.):
        initApplicationNative(screenWidth, screenHeight);

        // Create OpenGL ES view:
        int depthSize = 16;
        int stencilSize = 0;
        boolean translucent = QCAR.requiresAlpha();

        glView = new GLView(this);
        glView.init(QCAR.GL_11, translucent, depthSize, stencilSize);
   
        renderer = new WFRenderer();
        glView.setRenderer(renderer);
        
        guiManager = new GuiManager(this);
    }
	
	
    /** A helper for loading native libraries stored in "libs/armeabi*". */
    public static boolean loadLibrary(String libName){
        try{
            System.loadLibrary(libName);
            Log.e("WF","Native library lib" + libName + ".so loaded");
            return true;
        }
        catch (UnsatisfiedLinkError ulee){
        	Log.e("WF","The library lib" + libName +
                            ".so could not be loaded");
        }
        catch (SecurityException se){
            Log.e("WF","The library lib" + libName +
                            ".so was not allowed to be loaded");
        }
        return false;
    }
}
