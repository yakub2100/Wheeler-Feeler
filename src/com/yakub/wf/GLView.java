package com.yakub.wf;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.Log;

/**
 * 
 * @author yakub and QCAR
 *	this  class is largely copied from QCAR's ImageTargets example
 */
public class GLView extends GLSurfaceView{

	public GLView(Context context) {
		super(context);
	}

	public void init(int gl11, boolean translucent, int depthSize,int stencilSize) {
		
		//set translucent format to allow camera image to show through in the background
		if (translucent){
            this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        }
		// Setup the context factory GL rendering
        setEGLContextFactory(new WFContextFactory());
        
        //choose an EGLConfig that matches the format of the surface
        setEGLConfigChooser( translucent ?
                new ConfigChooser(8, 8, 8, 8, depthSize, stencilSize) :
                new ConfigChooser(5, 6, 5, 0, depthSize, stencilSize) );
	}
	
	/**
	 * Inner class, creates OpenGL context
	 * @author yakub
	 *
	 */
	private static class WFContextFactory implements GLSurfaceView.EGLContextFactory{
		
		private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
		
		@Override
		public EGLContext createContext(EGL10 egl, EGLDisplay dsp,EGLConfig cfg) {
			
			EGLContext context;
			
			Log.d("WF","Creating OpenGL ES 1.x context");
			
            checkEglError("Before eglCreateContext", egl);
            //create context
            int[] atribs = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL10.EGL_NONE};
            context = egl.eglCreateContext(dsp, cfg,EGL10.EGL_NO_CONTEXT, atribs);
			
            checkEglError("After eglCreateContext", egl);
            return context;
		}
	
		@Override
		public void destroyContext(EGL10 egl, EGLDisplay display,EGLContext context) {
			egl.eglDestroyContext(display, context);
		}
	}
	
	 /** Checks the OpenGL error. */
    private static void checkEglError(String prompt, EGL10 egl){
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS){
            Log.e("WF",String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }
    
    /** The config chooser. */
    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser{
    	
    	// Subclasses can adjust these values:
        protected int redSize;
        protected int greenSize;
        protected int blueSize;
        protected int alphaSize;
        protected int depthSize;
        protected int stencilSize;
        private int[] value = new int[1];
    	
        public ConfigChooser(int r, int g, int b, int a, int depth, int stencil){
            redSize = r;
            greenSize = g;
            blueSize = b;
            alphaSize = a;
            depthSize = depth;
            stencilSize = stencil;
        }


        private EGLConfig getMatchingConfig(EGL10 egl, EGLDisplay display,int[] configAttribs){
            
        	// Get the number of minimally matching EGL configurations
            int[] num_config = new int[1];
            egl.eglChooseConfig(display, configAttribs, null, 0, num_config);

            int numConfigs = num_config[0];
            if (numConfigs <= 0)
                throw new IllegalArgumentException("No matching EGL configs");

            // Allocate then read the array of minimally matching EGL configs
            EGLConfig[] configs = new EGLConfig[numConfigs];
            egl.eglChooseConfig(display, configAttribs, configs, numConfigs,
                num_config);

            // Now return the "best" one
            return chooseConfig(egl, display, configs);
        }

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display){
            
            final int EGL_OPENGL_ES1X_BIT = 0x0001;
            final int[] s_configAttribs_gl1x = {
                EGL10.EGL_RED_SIZE, 5,
                EGL10.EGL_GREEN_SIZE, 6,
                EGL10.EGL_BLUE_SIZE, 5,
                EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES1X_BIT,
                EGL10.EGL_NONE
            };

            return getMatchingConfig(egl, display, s_configAttribs_gl1x);
        }


        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display, EGLConfig[] configs){
            for(EGLConfig config : configs){
                int d = findConfigAttrib(egl, display, config,
                        EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(egl, display, config,
                        EGL10.EGL_STENCIL_SIZE, 0);

                // We need at least mDepthSize and mStencilSize bits
                if (d < depthSize || s < stencilSize)continue;

                // We want an *exact* match for red/green/blue/alpha
                int r = findConfigAttrib(egl, display, config,
                        EGL10.EGL_RED_SIZE, 0);
                int g = findConfigAttrib(egl, display, config,
                            EGL10.EGL_GREEN_SIZE, 0);
                int b = findConfigAttrib(egl, display, config,
                            EGL10.EGL_BLUE_SIZE, 0);
                int a = findConfigAttrib(egl, display, config,
                        EGL10.EGL_ALPHA_SIZE, 0);

                if (r == redSize &&
                    g == greenSize &&
                    b == blueSize &&
                    a == alphaSize)
                    return config;
            }
            return null;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display, 
        		EGLConfig config, int attribute,int defaultValue){

            if (egl.eglGetConfigAttrib(display, config, attribute, value))
                return value[0];

            return defaultValue;
        }
    }
}
