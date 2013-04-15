package com.yakub.wf;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import com.qualcomm.QCAR.QCAR;

import android.opengl.GLSurfaceView;

/**
 * Renderer class
 * @author yakub
 *
 */
public class WFRenderer implements GLSurfaceView.Renderer{
	
	public boolean isActive=false;
	
	/** Native function for initializing the renderer. */
    public native void initRendering();

    /** Native function to update the renderer. */
    public native void updateRendering(int width, int height);
    
    /** The native render function. */
    public native void renderFrame();
    
    /** Native method for setting / updating the projection matrix
     * for AR content rendering */
    private native void setProjectionMatrix();
	
	@Override
	public void onDrawFrame(GL10 arg0) {
		
		//return if rendering is off
		if (!isActive)return;
		
		// Update projection matrix:
        setProjectionMatrix();
        //native render function
        renderFrame();	
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
      
		//native render update
        updateRendering(width, height);
        // Call QCAR function to handle render surface size changes:
        QCAR.onSurfaceChanged(width, height);
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		
		//native rendering init
        initRendering();
        // Call QCAR function to (re)initialize rendering after first use
        // or after OpenGL ES context was lost (e.g. after onPause/onResume):
        QCAR.onSurfaceCreated();	
	}
}
