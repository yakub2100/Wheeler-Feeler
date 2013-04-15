package com.yakub.wf;

import yuku.ambilwarna.AmbilWarnaDialog;
import yuku.ambilwarna.AmbilWarnaDialog.OnAmbilWarnaListener;
import com.yakub.wf.R;
import android.content.Context;
import android.graphics.Color;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RadioButton;

public class GuiManager {
	
	//menu buttons
    private RadioButton w1;
    private RadioButton w2;
    private RadioButton w3;
    private RadioButton w4;
    private Button pb;
    private Button plus;
    private Button minus;
    
    //listener for buttons
    OnClickListener lis;
    
    private View guiView;
    
    private int currentColor=0xff000000;
    
    
    //native method to send model ID to native for selection
    private native void setModel(int i);
    
    //native method to set model's color
    private native void setModelColor(float r,float g,float b);
    
    //native scaling functions
    private native void scaleUp();
    private native void scaleDown();
    
    Context c;
    
    public GuiManager(Context c){
    	this.c=c;
    	//add GUI view on top of camera view
    	guiView=View.inflate(c, R.layout.gui_layout, null);
    }
    
    /**
     * Method to setup the GUI
     * assigns listeners to buttons
     */
    public void setupGui(){
    	
    	  w1 = (RadioButton) guiView.findViewById(R.id.w1);   		  
    	  w2 = (RadioButton) guiView.findViewById(R.id.w2);
    	  w3 = (RadioButton) guiView.findViewById(R.id.w3);
    	  w4 = (RadioButton) guiView.findViewById(R.id.w4);
    	  pb = (Button) guiView.findViewById(R.id.paint_button);
    	  plus = (Button) guiView.findViewById(R.id.plus_button);
    	  minus = (Button) guiView.findViewById(R.id.minus_button);
    	  
 	  
    	  //model select listener
    	  lis =new View.OnClickListener(){
    		  
              public void onClick(View v){
            	  RadioButton b = (RadioButton)v;
            	  String tmp=b.getHint().toString();
            	  int i = Integer.parseInt(tmp);
            	  setModel(i);  
              }
          };
          w1.setOnClickListener(lis);
          w2.setOnClickListener(lis);
          w3.setOnClickListener(lis);
          w4.setOnClickListener(lis);
          
          w1.setHintTextColor(Color.TRANSPARENT);
          w2.setHintTextColor(Color.TRANSPARENT);
          w3.setHintTextColor(Color.TRANSPARENT);
          w4.setHintTextColor(Color.TRANSPARENT);
          
          w1.setChecked(true);
          
          //paint button listener
          pb.setOnClickListener(new View.OnClickListener(){
              public void onClick(View v){       	 
            	  paintWheel();	  
              }
          });  
          
          //plus button listener
          plus.setOnClickListener(new View.OnClickListener(){
              public void onClick(View v){       	 
            	  scaleUp();	  
              }
          });  
          
          //minus button listener
          minus.setOnClickListener(new View.OnClickListener(){
              public void onClick(View v){       	 
            	  scaleDown();	  
              }
          }); 
    }
    
    /**
     * Method reads color from Youku's color picker
     * and calls native function to change model's color
     */
    public void paintWheel() {
    	
        //initialColor is the initially-selected color to be shown in the rectangle on the left of the arrow.
        //for example, 0xff000000 is black, 0xff0000ff is blue. Please be aware of the initial 0xff which is the alpha.
        AmbilWarnaDialog dialog = new AmbilWarnaDialog(c, currentColor, new OnAmbilWarnaListener() {
 
            // Executes, when user click Cancel button
            @Override
            public void onCancel(AmbilWarnaDialog dialog){
            }
 
            // Executes, when user click OK button
            @Override
            public void onOk(AmbilWarnaDialog dialog, int color) {
            	
            	currentColor=color;
            	//convert RGB values to float(0-1) for OpenGL use
            	float r=(float)Color.red(color)/255;
            	float g=(float)Color.green(color)/255;
            	float b=(float)Color.blue(color)/255;
            	
            	setModelColor(r,g,b);
            }
        });
        dialog.show();
    }
    
    /**
     * Method to get the GUI view 
     * used in main activity o setup the overlay GUI
     * @return View
     */
    public View getGuiView(){
    	
    	return guiView;
    }
}
