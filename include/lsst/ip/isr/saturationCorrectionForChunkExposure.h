// -*- LSST-C++ -*- // fixed format comment for emacs
/**
  * \file
  *
  * \ingroup isr
  *
  * \brief Implementation of the templated Instrument Signature Removal 
  * stage of the nightly LSST Image Processing Pipeline.
  *
  * \author Nicole M. Silvestri, University of Washington
  *
  * Contact: nms@astro.washington.edu
  *
  * \version
  *
  * LSST Legalese here...
  */
	
#ifndef LSST_IP_ISR_SATURATIONCORRECTIONFORCHUNKEXPOSURE_H
#define LSST_IP_ISR_SATURATIONCORRECTIONFORCHUNKEXPOSURE_H
	
#include <vector>

#include <lsst/daf/base.h>
#include <lsst/daf/data/LsstBase.h>	
#include <lsst/afw/image/Exposure.h>
#include <lsst/pex/policy/Policy.h>

/** \brief Remove all non-astronomical source counts from the Chunk Exposure's
  * pixels.
  * 
  * The sequence of sub-stages within the Instrument Signature Removal (ISR)
  * stage of the nightly IPP are as follows:
  * 
  * Saturation Correction for Chunk Exposure
  * Overscan Correct and Trim Chunk Exposure
  * Bias Correct Chunk Exposure
  * Dark Current Correct Chunk Exposure
  * Linearize Chunk Exposure
  * Flat Field Correct Chunk Exposure
  *  (DC3 Stretch) - Pupil Image Correction
  *  - Illumination Correction (scattered light correction)
  * Defringe Chunk Exposure
  * (DC3 Stretch) Geometric Distortion Correction
  * (DC3 Stretch) Mask and Correct Additional Artifacts
  * (DC3 Stretch) Additional Flat Correction
  * (DC3 Stretch) Crosstalk Correct Chunk Exposure
  * (DC3 Stretch) Cosmic Ray Detection
  * 
  * Crosstalk Correction will be incorporated as an individual sub-stage for the
  * Data Release ISR stage.  It is not currently part of the nightly ISR stage
  * as we receive a crosstalk corrected image from the camera for the nightly
  * pipeline.
  * 
  */
	
namespace lsst {
namespace ip {
namespace isr {
	       
    typedef boost::uint16_t maskPixelType;
    
    template<typename ImageT, typename MaskT>
    lsst::afw::image::Exposure<ImageT, MaskT> saturationCorrectionForChunkExposure(
        lsst::afw::image::Exposure<ImageT, MaskT> &chunkExposure,
        lsst::pex::policy::Policy &isrPolicy,
	lsst::pex::policy::Policy &datasetPolicy
//        std::vector<float> &saturationLookupTable
        );
    


}}} // namespace lsst::ip::isr
	
#endif // !defined(LSST_IP_ISR_SATURATIONCORRECTIONFORCHUNKEXPOSURE_H)
