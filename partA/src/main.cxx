// main.cxx : Defines the entry point for the console application.
//
// This is the top-level function of an example program that displays a VXL
// image on an fltk OpenGL panel

#include "VisCompUI.h"

// include various VXL include files needed for the application
#include "vxl_includes.h"

// code for loading VXL images
#include "file/load_image.h"

// code that implements the matting computations
#include "matting/matting.h"

// code that implements pyramid operations
#include "pyramid/pyramid.h"

// code the implements morphing operations
#include "morphing/morphing.h"


// Routine for processing the command-line arguments (defined below)
// It returns false if the program should exit immediately after this
// function returns
bool process_args(int argc, char** argv, matting& M, inpainting* I,
				  blending* B, morphing* Mrph)
{
     // What follows is the argument definition for the program
     // in the format required for automatic processing with vul_arg
	 //
     // First, we create a list to contain the descriptions for
     // each argument.
	 //
	 vul_arg_info_list arg_list;

     // Then, for each argument in the program, we add a description
     // to the list with
     //
     // vul_arg<type> var_name(list_name,"flag","Description",default_val);
     //
     // where "flag" is the flag name for a given argument, for example
     // "-cmd" or "-im", "Description" is a small text description of
     // the argument, and default_value is what gets assigned to the
     // corresponding variable if the argument is not found in the command
     // line

     // we want to create a command with the following command-line options:
	 //     -no_gui                do not open any fltk windows (non-interactive mode)
	 //     -matting               run the matting algorithm (this is the only algorithm we have for now)
	 //     -mback1 <filename>     input image that will serve as Background 1
	 //     -mcomp1 <filename>                                    Composite 1
	 //     -mback2 <filename>                                    Background 2
	 //     -mcomp2 <filename>                                    Composite 2
	 //     -mobj   <filename>     output image that will hold computed Object image
	 //     -malpha <filename>                                          alpha image
	 //     -mnewback <filename>   input image that will be used for compositing
	 //     -mnewcomp <filename>   output of the compositing operation
	 //     -help                  a brief description of command-line options
	 // additional options for inpainting algorithm:
	 //     -inpainting            run the inpainting algorithm
	 //     -isource <filename>    image that will serve as the source for inpainting
	 //     -imask   <filename>    binary image that will serve as the mask (ie. all pixels with intensity 0 will be inpainted) 
	 //     -iradius <radius>      radius of the patch used for inpainting
	 //     -inpaint <filename>    the file where the inpainted image should be stored
	 //     -iiter   <num>         the maximum number of iterations the algorithm should run

	 vul_arg<bool> no_gui(arg_list, "-no_gui", "Do not open any fltk windows (non-interactive mode)", false);
	 vul_arg<bool> mmatting(arg_list, "-matting", "Run the matting algorithm", false);
     vul_arg<vcl_string> mback1(arg_list,"-mback1","Input image that will serve as Background 1","");
     vul_arg<vcl_string> mcomp1(arg_list,"-mcomp1","Input image that will serve as Composite 1","");
     vul_arg<vcl_string> mback2(arg_list,"-mback2","Input image that will serve as Background 2","");
     vul_arg<vcl_string> mcomp2(arg_list,"-mcomp2","Input image that will serve as Composite 2","");
     vul_arg<vcl_string> mobj(arg_list,"-mobj","Output Object image","");
     vul_arg<vcl_string> malpha(arg_list,"-malpha","Output Alpha matte","");
     vul_arg<vcl_string> mnewback(arg_list,"-mnewback","Input image that will serve as the new Background","");
     vul_arg<vcl_string> mnewcomp(arg_list,"-mnewcomp","Output image that will hold the new Composite","");

	 // inpainting options
	 vul_arg<bool> iinpainting(arg_list, "-inpainting", "Run the inpainting algorithm", true);
	 vul_arg<vcl_string> isource(arg_list,"-isource","Input image that will serve as Source","");
	 vul_arg<vcl_string> imask(arg_list,"-imask","Input image that will server as Initial Mask","");
	 vul_arg<vcl_string> iinpainted(arg_list,"-inpaint","The resulting Inpainted image");
	 // by default, we use a patch radius of 4 
	 vul_arg<int> iradius(arg_list,"-iradius","Patch radius for inpainting", 4);
	 // by default, we run the algorithm to completion
	 vul_arg<int> niters(arg_list,"-iiter","Number of iterations to run", 0);

	 // morphing options
	 vul_arg<bool> morph(arg_list, "-morphing", "Run the morphing algorithm", false);
	 vul_arg<vcl_string> msource0(arg_list,"-msource0","Input image that will serve as Source I0","");
	 vul_arg<vcl_string> msource1(arg_list,"-msource1","Input image that will serve as Source I1","");
	 vul_arg<vcl_string> mbase(arg_list, "-mbase","The basename of the resulting image set (ie. no extension)","");
	 vul_arg<bool> mwarp(arg_list, "-mwarp","Save the warped I0 and I1 images","");
	 vul_arg<double> ma(arg_list, "-ma","The a parameter",morphing::get_a_default());
	 vul_arg<double> mb(arg_list, "-mb","The b parameter",morphing::get_b_default());
	 vul_arg<double> mp(arg_list, "-mp","The p parameter",morphing::get_p_default());
	 vul_arg<double> mt(arg_list, "-mt","The t parameter",morphing::get_t_default());
	 vul_arg<int> mnum(arg_list, "-mnum","The number of intermediate images to generate", morphing::get_num_images_default());
	 vul_arg<vcl_string> mlines(arg_list,"-mlines","The file containing line pairs","");

	 // blending options
	 vul_arg<bool> blend(arg_list, "-blending", "Run the pyramid blending algorithm", false);
	 vul_arg<vcl_string> bsource0(arg_list,"-bsource0","Input image that will serve as Source0","");
	 vul_arg<vcl_string> bsource1(arg_list,"-bsource1","Input image that will serve as Source1","");
	 vul_arg<vcl_string> bmask(arg_list,"-bmask","Input image that will serve as Mask","");
	 vul_arg<vcl_string> bblend(arg_list,"-bblend","Output base filename for blended image","");
	 vul_arg<bool> bgpyr0(arg_list, "-bgpyr0", "Save the gauss pyramid of Source0", false);
	 vul_arg<bool> bgpyr1(arg_list, "-bgpyr1", "Save the gauss pyramid of Source1", false);
	 vul_arg<bool> bgpyrm(arg_list, "-bgpyrm", "Save the gauss pyramid of Mask", false);
	 vul_arg<bool> bgpyrb(arg_list, "-bgpyrb", "Save the gauss pyramid of the Blended image", false);
	 vul_arg<bool> blpyr0(arg_list, "-blpyr0", "Save the Laplacian pyramid of Source0", false);
	 vul_arg<bool> blpyr1(arg_list, "-blpyr1", "Save the Laplacian pyramid of Source1", false);
	 vul_arg<bool> blpyrm(arg_list, "-blpyrm", "Save the Laplacian pyramid of Mask", false);
	 vul_arg<bool> blpyrb(arg_list, "-blpyrb", "Save the Laplacian pyramid of the Blended image", false);

    
     // Now set the switch for the help option

     arg_list.set_help_option("-help");

     // Once we've added the argument descriptions, we call on the
     // lists parse() function to do the work! the third argument
     // in the call to parse() is a boolean flag, when set to 1,
     // the parse function warns of un-recognized commands.

     arg_list.parse(argc,argv,(bool)1);

     // After this is done, the variables will contain either the
     // values entered by the user, or the defaults. One can check
     // whether a particular argument was found in the command
     // line by checking the appropriate variable's set() function
     // as shown below... 
     //
	 // Note that to access the VALUE of a given argument we use the
	 // notation arg_name()


/*	 if (warping.set() == true) {
		 vil_image_view<vxl_byte> source1 = load_image1(wsource1());
		 vil_image_view<vxl_byte> source2 = load_image1(wsource2());
		 vil_image_view<vxl_byte> mask = load_image1(wmask());
		 vil_image_view<vxl_byte> blended;
		 vil_image_view<vxl_byte> blended2;
		 blend(source1, source2, mask, blended);
		 blend2(source1, source2, mask, blended2);

		 pyramid P1(source1);
		 pyramid P2(source2);
		 pyramid M(mask);
		 for (int l=0; l<P1.N(); l++) {
			 vil_image_view<vxl_byte> im;
			 char name[80];
			 P1.g(l,0,im);
			 sprintf(name,"s1level_exp.%d.tif\0",l);
			 vil_save(im,name);
			 sprintf(name,"s1level.%d.tif\0",l);
			 P1.g(l,l,im);
			 vil_save(im,name);
			 P2.g(l,0,im);
			 sprintf(name,"s2level_exp.%d.tif\0",l);
			 vil_save(im,name);
			 sprintf(name,"s2level.%d.tif\0",l);
			 P2.g(l,l,im);
			 vil_save(im,name);
			 M.g(l,0,im);
			 sprintf(name,"mlevel_exp.%d.tif\0",l);
			 vil_save(im,name);
			 sprintf(name,"mlevel.%d.tif\0",l);
			 M.g(l,l,im);
			 vil_save(im,name);
		 }
		 vil_save(P1.pack_gauss(), "s1packg.tif");
		 vil_save(P1.pack_laplacian(), "s1packl.tif");
		 vil_save(P2.pack_gauss(), "s2packg.tif");
		 vil_save(P2.pack_laplacian(), "s2packl.tif");
	 }
	 */

	 //
	 //    we will process the morphing arguments first
	 //

	 if (morph.set() == true) {
		 // did the user supply an image I0?
		 if (msource0.set() == true) {
			 vcl_cerr << "process_args(): loading input image(s) ..." << vcl_endl;
			 if (Mrph->set(Mrph->I0, load_image(msource0())) == false) {
				vcl_cerr << "morphing::set: error reading image " << msource0() << vcl_endl;
				return false;
			 }
		 }
		 // did the user supply an image I1?
		 if (msource1.set() == true) {
			 vcl_cerr << "process_args(): loading input image(s) ..." << vcl_endl;
			 if (Mrph->set(Mrph->I1, load_image(msource1())) == false) {
				vcl_cerr << "morphing::set: error reading image " << msource1() << vcl_endl;
				return false;
			 }
		 }
		 // did the user supply a lines file?
		 if (mlines.set() == true) {
			 vcl_string str = mlines();
			 vcl_cerr << "process_args(): loading line pairs ..." << vcl_endl;
			 if (Mrph->load_linepairs(str.c_str()) == false) {
				 vcl_cerr << "morphing::load_linepairs::load: error loading line pairs file " << mlines() << vcl_endl;
				 return false;
			 }
		 }

		 // set the algorithm's parameters
		 Mrph->set_a(ma());
		 Mrph->set_b(mb());
		 Mrph->set_p(mp());
		 Mrph->set_t(mt());
		 Mrph->set_num_images(mnum());
		 // set the output filenames to use
		 if (mbase.set() == true) {
			 Mrph->set_morph_basename(mbase());
			 Mrph->toggle_write_morph();
		 }
		 if (mwarp.set() == true)
			 // by default, we don't write images to disk
			 // so we need to change that mode if this flag is set
			 Mrph->toggle_write_warped();

		 // if both source images and a lines file are given, the algorithm is run
		 // automatically
		 if (msource0.set() && msource1.set() && mlines.set()) {

			vcl_cerr << "process_args(): morphing the images..." << vcl_endl;

			if (Mrph->compute() == false) {
				// there was a problem in the computation, so we do not continue
				// the non-interactive processing
				vcl_cerr << "morphing::compute(): An error occured -- exiting" << vcl_endl;
				return (no_gui.set() == false);
			}
		}
	 }

	 //
	 //    now we process the blending arguments
	 //
	 if (blend.set() == true) {
		 // did the user supply an image for Source0?
		 if (bsource0.set() == true) {
			 vcl_cerr << "process_args(): loading input image(s) ..." << vcl_endl;
			 if (B->set(B->Source0, load_image(bsource0())) == false) {
				vcl_cerr << "blending::set: error reading image " << bsource0() << vcl_endl;
				return false;
			 }
		 }
		 // did the user supply an image for Source1?
		 if (bsource1.set() == true) {
			 vcl_cerr << "process_args(): loading input image(s) ..." << vcl_endl;
			 if (B->set(B->Source1, load_image(bsource1())) == false) {
				vcl_cerr << "blending::set: error reading image " << bsource1() << vcl_endl;
				return false;
			 }
		 }
		 // did the user supply an image for the Mask?
		 if (bmask.set() == true) {
			 vcl_cerr << "process_args(): loading input image(s) ..." << vcl_endl;
			 if (B->set(B->Mask, load_image1(bmask())) == false) {
				vcl_cerr << "blending::set: error reading image " << bmask() << vcl_endl;
				return false;
			 }
		 }
		 // if all 3 input images are provided, we run the blending algorithm automatically
		 if (bsource0.set() && bsource1.set() && bmask.set()) {

			 vcl_cerr << "process_args(): blending the images..." << vcl_endl;

			if (B->compute() == false) {
				// there was a problem in the computation, so we do not continue
				// the non-interactive processing
				vcl_cerr << "blending::compute(): An error occured -- exiting" << vcl_endl;
				return (no_gui.set() == false);
			}

		 }
		 // if the user has specified any output flags,  we write the results to disk
		 if (bblend.set() == true) {
			 // write the blending result
			 if (B->save_blended(bblend().c_str()) == false)
				 // the vcl_endl just adds a linebreak to the stream 
				 vcl_cerr << "vil_save: error occured while saving blended image" << vcl_endl;
		 }
		 //
		 // saving the Gauss pyramids
		 //
		 // write the source0 pyramid
		 if (bgpyr0.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Source0, true, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Source0, true, "Gauss");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving source0 pyramid" << vcl_endl;
		 }
		 // write the source1 pyramid
		 if (bgpyr1.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Source1, true, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Source1, true, "Gauss");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving source1 pyramid" << vcl_endl;
		 }
		 // write the mask pyramid
		 if (bgpyrm.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Mask, true, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Mask, true, "Gauss");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving mask pyramid" << vcl_endl;
		 }
		 // write the blended image pyramid
		 if (bgpyrb.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Blend, true, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Blend, true, "Gauss");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving blended image pyramid" << vcl_endl;
		 }
		 //
		 // saving the Laplacian pyramids
		 //
		 // write the source0 pyramid
		 if (bgpyr0.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Source0, false, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Source0, false, "Laplacian");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving source0 pyramid" << vcl_endl;
		 }
		 // write the source1 pyramid
		 if (bgpyr1.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Source1, false, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Source1, false, "Laplacian");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving source1 pyramid" << vcl_endl;
		 }
		 // write the mask pyramid
		 if (bgpyrm.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Mask, false, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Mask, false, "Laplacian");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving mask pyramid" << vcl_endl;
		 }
		 // write the blended image pyramid
		 if (bgpyrb.set() == true) {
			 bool ok;
			 if (bblend.set() == true)
				 ok = B->save_pyramid(B->Blend, false, bblend().c_str());
			 else
				 ok = B->save_pyramid(B->Blend, false, "Laplacian");
			 if (!ok)
				 vcl_cerr << "vil_save: error occured while saving blended image pyramid" << vcl_endl;
		 }
	 }

	 //
	 //    now we process the inpainting arguments
	 //

	 if (iinpainting.set() == true) {
		 // did the user supply a Source image?
		 if (isource.set() == true) {
			 vcl_cerr << "process_args(): loading input image(s) ..." << vcl_endl;
			 if (I->set(I->Source, load_image(isource())) == false) {
				vcl_cerr << "inpainting::set: error reading image " << isource() << vcl_endl;
				return false;
			 }
		 }
		 if (imask.set() == true) {
			 if (I->set(I->Init_Filled, load_image1(imask())) == false) {
				vcl_cerr << "inpainting::set: error reading image " << imask() << vcl_endl;
				return false;
			 }
		 }	

		 // set the patch size the default values
		 I->set_patch_radius(iradius());

		 // if both source and mask are given we run the inpainting algorithm
		 // automatically
		 vcl_cerr << "process_args(): inpainting image..." << vcl_endl;

		 // ok, we have all the input images, so let's run the algorithm
		 if (I->compute(niters()) == false) {
			 // there was a problem in the computation, so we do not continue
			 // the non-interactive processing
			 vcl_cerr << "inpainting::compute(): An error occured -- exiting" << vcl_endl;
			 return (no_gui.set() == false);
		 }

		 // ok, the processing was successful.
		 // now, if the user has specified output image files, write the
		 // result to disk

		 if (iinpainted.set() == true) {
			 vcl_cerr << "Writing the result to disk ...\n";

			 // write the inpainting result
			 // note that we need to use the method c_str()
			 // to convert a vcl_string to a C-type string (i.e. const char*)
			 if (vil_save((vil_image_view<vxl_byte>)I->get(I->Inpainted), iinpainted().c_str()) == false) 
				 // the vcl_endl just adds a linebreak to the stream 
				 vcl_cerr << "vil_save: error occured while saving object image" << vcl_endl;
		 } 
	 }

	 //
	 // now we process the matting arguments 
	 //

	 if (mmatting.set() == true) {

	 // 
	 // first, we load the input images (if any)
	 //
	 
	 // write a string to standard error 
	 // note that vcl_endl just adds a linebreak to the stream

	 // did the user supply an image for Background 1?
	 if (mback1.set() == true) {
		 vcl_cerr << "process_args(): loading input image(s) ..." << vcl_endl;
		 // if so, load it into the matting data structure
		 M.set(M.Back1, load_image(mback1()));
	 }
	 if (mback2.set() == true)
		 M.set(M.Back2, load_image(mback2()));
	 if (mcomp1.set() == true)
		 M.set(M.Comp1, load_image(mcomp1()));
	 if (mcomp2.set() == true)
		 M.set(M.Comp2, load_image(mcomp2()));
	 if (mnewback.set() == true)
		 M.set(M.NewBack, load_image(mnewback()));
		
	 //
	 // if all input images are given,
	 // we run the matting algorithm automatically
	 //
	 if ((mback1.set() == true) && (mback2.set() == true) &&
		 (mcomp1.set() == true) && (mcomp2.set() == true)) {

		 vcl_cerr << "process_args(): computing alpha matte..." << vcl_endl;

		 // ok, we have all the input images, so let's run the algorithm
		 if (M.compute() == false)
			 // there was a problem in the computation, so we do not continue
			 // the non-interactive processing
			 return (no_gui.set() == false);

		 // ok, the processing was successful.
		 // now, if the user has specified output image files, write the
		 // result to disk

		 if (mobj.set() == true) {
			 vcl_cerr << "Writing the result(s) to disk ...\n";

			 // write the object image
			 // note that we need to use the method c_str()
			 // to convert a vcl_string to a C-type string (i.e. const char*)
			 if (vil_save((vil_image_view<vxl_byte>)M.get(M.Object), mobj().c_str()) == false) 
				 // the vcl_endl just adds a linebreak to the stream 
				 vcl_cerr << "vil_save: error occured while saving object image" << vcl_endl;
		 } 
		 if (malpha.set() == true)
			 // write the object image
			 // note that we need to use the method c_str()
			 // to convert a vcl_string to a C-type string (i.e. const char*)
			 if (vil_save(M.get1(M.Alpha), malpha().c_str()) == false)
				 vcl_cerr << "vil_save: error occured while saving alpha image" << vcl_endl;

		 // finally, if the user has supplied a new background 
		 // image, perform a compositing operation
		 if (mnewback.set() == true) {
			 vil_image_view<vil_rgb<vxl_byte> > im1, im2;
			 // first load the image corresponding to the new background
			 im1 = load_image(mnewback());
			 // if the load was successful, run the compositing operation
			 if ((bool)im1 == true) {
				 vcl_cerr << "Computing a new composite ..." << vcl_endl;

				 if (M.compute_composite(im1, im2) == true) {
					 // if the compositing was successful and the user has
					 // supplied an output filename, we save the composite to 
					 // a file
					 if (mnewcomp.set() == true)
					      if (vil_save((vil_image_view<vxl_byte>)im2, mnewcomp().c_str()) == false)
							 vcl_cerr << "vil_save: error occured while saving new composite image" << vcl_endl; 
				 } else
					 vcl_cerr << "process_args():: an error occured during the compositing operation" << vcl_endl;
			 }
		 }

	 }
	 }

	 // if the user requested that no windows are opened we should
	 // tell main() that it should exit now
	 return (no_gui.set() == false);

}


// Program entry point
int main(int argc, char **argv) {
	matting M;

	inpainting* I = new inpainting();
	morphing* Mrph = new morphing();
	blending* B = new blending();

	// Get the command-line arguments (if any)  
	if (process_args(argc, argv, M, I, B, Mrph) == false)
		return 0;
	else {
		 //
		 // run in interactive mode---we have to create the 
		 // fltk interface, etc
		 //
		 // Initialize fluid
	     Fl::visual(FL_INDEX || FL_DOUBLE);
	
		 // Create the UI windows for the interface
	     VisCompUI *window = new VisCompUI();
		 window->set_matting(M);
		 // pass the UI panels to the inpainting interface
		 I->add_panels(window->left_panel, window->right_panel);
		 window->set_inpainting(I);
		 // pass the UI panels to the morphing interface
		 Mrph->add_panels(window->left_panel, window->right_panel);
		 window->set_morphing(Mrph);
		 // pass the UI panels to the blending interface
		 B->add_panels(window->left_panel, window->right_panel);
		 window->set_blending(B);

	     // Display the UI windows
	     window->show();
     

	     return Fl::run();
	 }
}


