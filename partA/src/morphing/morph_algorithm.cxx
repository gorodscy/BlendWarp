// DO NOT MODIFY ANYWHERE EXCEPT WHERE EXPLICITLY NOTED!!

#include "morphing.h"

// 
// Top-level morphing routine
//
// The routine runs the basic morphing algorithm several times
// in order to generate a sequence of intermediate morphed images  
// between t=0 and t=1. 
// 
// The number of intermediate images is controlled by the 
// num_images_ parameter of the morphing class and is user-selectable
//
bool morphing::compute()
{
	int iter;
	bool ok = true;

	// if num_images > 1, we compute the t_ parameter
	// automatically before executing the morph
	if (num_images_ > 1) 
		for (iter=0; (iter<num_images_) && (ok); iter++) {
			// set the t parameter value for this iteration
			set_t((iter+1)*1.0/(num_images_+1));		
	
			// compute the morph for this setting of the parameter
			vcl_cerr << "Computing morph for t=" << get_t() << "\n";
			ok = ok && morph_iteration(iter);
		} 
	else {
		// otherwise, we just run the algorithm once, for the current
		// setting of the t parameter
		vcl_cerr << "Computing morph for t=" << get_t() << "\n";
		ok = morph_iteration(0);
	}
	return ok;
}

// 
// Top-level routine for the creation of a single morphed
// image
//
// The routine checks whether a morph can be computed
// (eg. that both images I0 and I1 have been specified),
// calls the routine that implements the Beier-Neely
// morphing algorithm, and finally writes the results to
// disk, if specified by the user
//
// the variable iter runs between 0 and num_images_ and is
// used only for file numbering when writing images to 
// disk
// 
bool morphing::morph_iteration(int iter)
{
	if ((morph_computed_ == false) || (outdated_ == true)) {

		if (outdated_ == true) {
			// we need to recalculate everything
			morph_computed_ = false;
		}

		if (((bool) I0_ == false) || 
			((bool) I1_ == false))
			// we do not have enough information yet to run the
			// algorithm, we have nothing to do
			return false;

		// ok, we have enough information to proceed

		// allocate space for all images
		warped_I0_.set_size(I0_.ni(), I0_.nj());
		warped_I1_.set_size(I0_.ni(), I0_.nj());
		morph_.set_size(I0_.ni(), I0_.nj());

		// compute the morph
		compute_morph();

		// update the compute-related flags
		morph_computed_ = true;
		outdated_ = false;

	} else {
		// the results have already been computed, so
		// we have nothing to do
	}

	// write to disk
	if (write_morph_ == true) {
		//
		// build the file name in the form <basefilename>.XXX.jpg
		// where XXX is the zero-padded iteration number
		//
		char fname[256];
		vcl_ostringstream mfname(fname);
		mfname 
			<< morph_basename_ << "." 
			<< vcl_setfill('0') << vcl_setw(3) << iter 
			<< ".jpg" << vcl_ends;
		 
		// to save, we need to access a (char *) representation
		// of the output string stream
		vcl_cerr << "writing Morph to file:" 
			<< (mfname.str()).c_str() << "\n";
		vil_save((vil_image_view<vxl_byte>)morph_, 
			     (mfname.str()).c_str());
	}
	if (write_warped_ == true) {
		char fname0[256];
		char fname1[256];
		vcl_ostringstream w0fname(fname0);
		vcl_ostringstream w1fname(fname1);

		w0fname 
			<< morph_basename_ << "." 
			<< "W0." 
			<< vcl_setfill('0') << vcl_setw(3) << iter 
			<< ".jpg" << vcl_ends;
		w1fname 
			<< morph_basename_ << "." 
			<< "W1." 
			<< vcl_setfill('0') << vcl_setw(3) << iter 
			<< ".jpg" << vcl_ends;

		vcl_cerr << "writing WarpedI0 to file " 
			<< (w0fname.str()).c_str() << "\n";
		vil_save((vil_image_view<vxl_byte>)warped_I0_, 
			     (w0fname.str()).c_str());
		vcl_cerr << "writing WarpedI1 to file " 
			<< (w1fname.str()).c_str() << "\n";
		vil_save((vil_image_view<vxl_byte>)warped_I1_, 
			     (w1fname.str()).c_str());
	}

	return true;
}

//
// Top-level implementation of the Beier-Neely morphing
// algorithm
//
// The routine should call the field_warp() routine as
// a subroutine for warping the images stored in
// variables I0_ and I1_
// 
// Specifications:
//   Inputs: The routine should assume that the following
//           private class variables contain valid data:
//
//   * The two source images, I0_ and I1_
//   * I0I1_linepairs_ holds the set of corresponding
//     line pairs between images I0_ and I1_
//   * The class variables a_, b_, p_ holding the parameters
//     of the field warping algorithm
//   * The parameter t_ that determines the in between 
//     warp between images I0_ and I1_
//
//   Outputs: The following private class variables are 
//            assumed to have valid data after the routine
//            returns:
//
//   * warped_I0_: the image holding the result of applying
//                 the field warp algorithm to image I0_
//   * warped_I1_: the image holding the result of applying
//                 the field warp algorithm to image I1_
//   * morph_: the image holding the result morphing images
//             I0_ and I1_
//   * I0W0_linepairs_: the set of corresponding line pairs
//                      between images I0_ and warped_I0_

void morphing::compute_morph()
{
	/////////////////////////////////////////
	// PLACE YOUR CODE BETWEEN THESE LINES //
	/////////////////////////////////////////

	// dummy implementation

	// just copy the input images to the result variables
	vil_copy_deep(I0_, warped_I0_);
	vil_copy_deep(I1_, warped_I1_);
	vil_copy_deep(I1_, morph_);

	// create a linepair where bot the original and
	// the warped lines are identical and equal to the
	// user-specified lines in image I0
	I0W0_linepairs_ = I0I1_linepairs_.copy(0, 1);

	////////////////////////////////////////
}

// 
// Routine that implements the Beier-Neely field warping 
// algorithm
//
// Input variables:
//   source:      the vxl image to be warped
//   linepairs:   the set of corresponding line pairs between
//                the source image and the destination image
//   a,b,p:       the field warping parameters
//
// Output variables:
//   destination: the warped image. the routine should assume
//                that memory for this image has already been
//                allocated and that the image is of identical
//                size as the source image
// 
void morphing::field_warp(
		const vil_image_view<vil_rgb<vxl_byte> >& source, 
		linepairs& lps,
		double a, double b, double p,
		vil_image_view<vil_rgb<vxl_byte> >& destination)
{
	/////////////////////////////////////////
	// PLACE YOUR CODE BETWEEN THESE LINES //
	/////////////////////////////////////////

	////////////////////////////////////////
}

////////////////////////////////////////
// PLACE ANY ADDITIONAL CODE          //
// BETWEEN THESE LINES                //
////////////////////////////////////////

////////////////////////////////////////

