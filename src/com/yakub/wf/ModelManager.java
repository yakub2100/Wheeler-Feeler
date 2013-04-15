package com.yakub.wf;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import com.yakub.wf.R;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

/**
 * Helper class to deal with 3D model loading
 * @author yakub
 *
 */
public class ModelManager {
	
	private int modelCnt=0;
	
	/**
	 * Native method that reads models from SD card and prepares them for rendering
	 * @param s
	 */
	private native void loadModels(String s);
	
	/**
	 * Method to load 3D models
	 * extracts models to SD card, then calls native load method
	 * @param c
	 */
	public void loadModels(Context c){
				
		InputStream in = c.getResources().openRawResource(R.raw.wheel);
        copyAndLoadModel(in);
        in = c.getResources().openRawResource(R.raw.wheel2);
        copyAndLoadModel(in);
        in = c.getResources().openRawResource(R.raw.wheel3);
        copyAndLoadModel(in);
        in = c.getResources().openRawResource(R.raw.wheel4);
        copyAndLoadModel(in);
	}
	
	 /**
	  * Method extracts model onto SD card, passing file name to JNI to parse model
	  * @param in
	  */
    private void copyAndLoadModel(InputStream in){
    	
    	try{
    		 //create directory structure
	    	 File dir = new File(Environment.getExternalStorageDirectory() + java.io.File.separator + "models");
	    	 if(!dir.exists()){
	    		 dir.mkdirs();
	    	 }
	    	 
	    	 //create file
	    	 File file =new File(dir.getPath()+ java.io.File.separator + modelCnt + ".3DS");
	    	 if(!file.exists()){
	    		 file.createNewFile();
	    	 }
	    	 
	    	 //copy
	         FileOutputStream out = new FileOutputStream(file);
	         byte[] buff = new byte[1024];
	         int read = 0;
	
	         try { 
	        	 while ((read = in.read(buff)) > 0) {
	        		 out.write(buff, 0, read);
	        	 }
	        	 //call native method on copied model
	        	 loadModels(file.getPath());
				 modelCnt++;
					            
	         } finally {
	              in.close();
	              out.close();
	         }
	    }
	    catch(Exception e){
	    	Log.e("WF","error opening files");
	    }
    }

}
