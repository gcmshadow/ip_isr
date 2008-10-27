// -*- LSST-C++ -*- // fixed format comment for emacs
/**
  * \file
  *
  * \ingroup isr
  *
  * \brief Implementation of the templated subStage, Defringe Chunk
  * Exposure, of the Instrument Signature Removal stage for the LSST
  * Image Processing Pipeline.
  *
  * \author Nicole M. Silvestri, University of Washington
  *
  * Contact: nms@astro.washington.edu
  *
  * \version
  *
  * LSST Legalese here...
  */
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/format.hpp>

#include <lsst/afw/image/Exposure.h>
#include <lsst/afw/math/Function.h>
#include <lsst/afw/image/Mask.h>
#include <lsst/afw/image/MaskedImage.h>
#include <lsst/daf/base/DataProperty.h>
#include <lsst/pex/exceptions/Exception.h>
#include <lsst/pex/logging/Trace.h>
#include <lsst/pex/policy/Policy.h>

#include "lsst/ip/isr/isr.h"

/** \brief Remove overscan strip region and other non-illuminated edge pixels
  * from the Chunk Exposure.  Valid region to be retained is defined by the
  * DATASEC (defines the four corners of the valid region).
  */

template<typename ImageT, typename MaskT>
lsst::afw::image::Exposure<ImageT, MaskT> defringeChunkExposure(
    lsst::afw::image::Exposure<ImageT, MaskT> const &chunkExposure,   
    lsst::afw::image::Exposure<ImageT, MaskT> const &masterChunkExposure,
    lsst::pex::policy::Policy &isrPolicy, 
    lsst::pex::policy::Policy &datasetPolicy
    ) { 

    //Get the Chunk Images/Mask from the Chunk Exposure
    lsst::afw::image::MaskedImage<ImageT, MaskT> chunkMaskedImage = chunkExposure.getMaskedImage();
   
    //Get the Image Metadata from the Chunk Science Image
    lsst::daf::base::DataProperty::PtrType chunkMetadata = chunkMaskedImage.getImage->getMetadata();

//Get the Master Fringe Chunk Images/Mask from the Master Fringe Chunk Exposure
    lsst::afw::image::MaskedImage<ImageT, MaskT> masterChunkMaskedImage = masterChunkExposure.getMaskedImage();
   
    //Get the Image Metadata from the Chunk Science Image
    lsst::daf::base::DataProperty::PtrType masterChunkMetadata = masterChunkMaskedImage.getImage->getMetadata();

    // Check that this ISR sub-stage has not been run previously on this Chunk
    // Exposure.  If it has, terminate the stage.
    lsst::daf::base::DataProperty::PtrType isrFringeField = chunkMetadata->findUnique("ISR_FRINGECOR");
    if (isrFringeField) {
        lsst::pex::logging::TTrace<3>(std::string("In ") + __func__ + std::string(": Exposure has already been Fringe Corrected.  Terminating ISR sub-stage for this Chunk Exposure."));
        throw lsst::pex::exceptions::Runtime(std::string("Fringe correction previously performed."));
    }

    // Check that the Master Fringe Chunk Exposure and Chunk Exposure are
    // the same size.

    const int numCols = static_cast<int>(chunkExposure.getCols());
    const int numRows = static_cast<int>(chunkExposure.getRows()); 

    const int mnumCols = static_cast<int>(masterChunkExposure.getCols());
    const int mnumRows = static_cast<int>(masterChunkExposure.getRows()); 

    if (numCols != mnumCols || numRows != mnumRows) {
        throw lsst::pex::exceptions::LengthError(std::string("In ") + __func__ + std::string(": Chunk Exposure and Master Fringe Chunk Exposure are not the same size."));
    }

    // Check that the Master Fringe Chunk Exposure and Chunk Exposure are
    // derived from the same pixels.

    lsst::pex::policy::Policy fringePolicy = isrPolicy.getPolicy("fringePolicy");
    std::string chunkType = fringePolicy.getString("chunkType");
    if (chunkType = "amp") {
        lsst::daf::base::DataProperty::PtrType ampidField = chunkMetadata->findUnique("AMPID");
        unsigned int ampid;
        if (ampidField) {
            ampid = boost::any_cast<const int>(ampidField->getValue());
            return ampid;
        } else {
            throw lsst::pex::exceptions::NotFound(std::string("In ") + __func__ + std::string(": Could not get AMPID from the Chunk Metadata."));
        }
   
        lsst::daf::base::DataProperty::PtrType mampidField = masterChunkMetadata->findUnique("AMPID");
        unsigned int mampid;
        if (mampidField) {
            mampid = boost::any_cast<const int>(mampidField->getValue());
            return mampid;
        } else {
            throw lsst::pex::exceptions::NotFound(std::string("In ") + __func__ + std::string(": Could not get AMPID from the Master Fringe Chunk Metadata."));
        }
   
        if (ampid != mampid) {
            throw lsst::pex::exceptions::RangeError(std::string("In ") + __func__ + std::string(": Chunk Exposure and Master Fringe Chunk Exposure are not derived from the same pixels."));
        }
    // CHECK IT IF ITS A CCD 
    } else if (chunkType = "ccd") {
        lsst::daf::base::DataProperty::PtrType ccdidField = chunkMetadata->findUnique("CCDID");
        unsigned int ccdid;
        if (ccdidField) {
            ccdid = boost::any_cast<const int>(ccdidField->getValue());
            return ccdid;
        } else {
            throw lsst::pex::exceptions::NotFound(std::string("In ") + __func__ + std::string(": Could not get CCDID from the Chunk Metadata."));
        }
   
        lsst::daf::base::DataProperty::PtrType mccdidField = masterChunkMetadata->findUnique("CCDID");
        unsigned int mccdid;
        if (mccdidField) {
            mccdid = boost::any_cast<const int>(mccdidField->getValue());
            return mccdid;
        } else {
            throw lsst::pex::exceptions::NotFound(std::string("In ") + __func__ + std::string(": Could not get CCDID from the Master Fringe Chunk Metadata."));
        }
   
        if (ccdid != mccdid) {
            throw lsst::pex::exceptions::RangeError(std::string("In ") + __func__ + std::string(": Chunk Exposure and Master Fringe Chunk Exposure are not derived from the same pixels."));
        }
    } else {
        // check for raft-level compliance
        // not yet implemented
    }

   // Check that the Master Chunk Exposure and Fringe Chunk Exposure are taken
   // in the same filter

    lsst::daf::base::DataProperty::PtrType filterField = chunkMetadata->findUnique("FILTER");
    unsigned int filter;
    if (filterField) {
        //  Determine if the filter field value is a number (1-6?)or a string
        //  (ugrizY?)
        filter = boost::any_cast<const int>(filterField->getValue());
    } else {
        throw lsst::pex::exceptions::NotFound(std::string("In ") + __func__ + std::string(": Could not get FILTER from the Chunk Metadata."));
    }    
    if (isalpha(filter)) 
       lsst::pex::logging::TTrace<3>(std::string("In ") + __func__ + std::string(": Filter Name: %s", filter)); 
//    } else if {
//        filter equal to LSST numerical designations for filters ...do something else

     lsst::daf::base::DataProperty::PtrType mfilterField = masterChunkMetadata->findUnique("FILTER");
     unsigned int mfilter;
    if (mfilterField) {
        //  Determine if the filter field value is a number (1-6?)or a string (ugrizY?) 
        mfilter = boost::any_cast<const int>(mfilterField->getValue());
    } else {
        throw lsst::pex::exceptions::NotFound(std::string("In ") + __func__ + std::string(": Could not get FILTER from the Master Flat Field Chunk Metadata."));
    }    
    if (isalpha(mfilter)) 
        lsst::pex::logging::TTrace<3>(std::string("In ") + __func__ + std::string(": Filter Name: %s", mfilter)); 
            
    if (mfilterField) {
    
        // assuming that the filter will be a number for lsst (1-6) not a
        // string(ugrizY)??

        mfilter = boost::any_cast<const int>(mfilterField->getValue());
        std::string mfilter = boost::any_cast<const std::string>(mfilterField->getValue());
        return mfilter;
    } else {
        throw lsst::pex::exceptions::NotFound(std::string("In ") + __func__ + std::string(": Could not get AMPID from the Master Flat Field Chunk Metadata."));
    }
   
    if (filter != mfilter) {
        throw lsst::pex::exceptions::DomainError(std::string("In ") + __func__ + std::string(": Chunk Exposure and Master Flat Field Chunk Exposure are not from the same FILTER."));
    }

    // Parse the main ISR Policy file for Fringing sub-stage parameters.  

    // THIS IS PROBABLY DERIVED FROM SEVERAL PLACES ON THE MASTER FRINGE. THIS
    // NEEDS MORE WORK HERE THAN JUST GRABBING A SCALE FACTOR

    double fringeScale = fringePolicy.getDouble("fringeScale");

    // do we need to preserve dynamic range by stretching 65K ADU by some
    // factor??
    double stretchFactor = fringePolicy.getDouble("stretchFactor");
    masterChunkExposure *= stretchFactor;
    bool sigClip = fringePolicy.getBool("sigClip");
    double sigClipVal = fringePolicy.getDouble("sigClipVal");

    // Divide the Chunk Exposure by the scaled Fringe Chunk
    // Exposure.  Hopefully RHL has fixed the Image class so that it properly
    // computes the varaince...

    if (fringeScale) {
        masterChunkExposure *= fringeScale;
        chunkExposure /= masterChunkExposure;
    } else {
        chunkExposure /= masterChunkExposure;
    }
    
    //Record the sub-stage provenance to the Image Metadata
     chunkMetadata->addProperty(lsst::daf::base::DataProperty("ISR_FRINGECOR", "Complete"));
    chunkMaskedImage.setMetadata(chunkMetadata);

    //Calculate additional SDQA Metrics here. 
            

    //Issue a logging message if the sub-stage executes without issue
    lsst::pex::logging::TTrace<7>(std::string("ISR sub-stage") + __func__ + std::string("completed successfully."));
         
}

/************************************************************************/
/* Explicit instantiations */

// template
// lsst::afw::image::Exposure<float, lsst::afw::image::maskPixelType> defringeChunkExposure(
//     lsst::afw::image::Exposure<float, lsst::afw::image::maskPixelType> const &chunkExposure,   
//     lsst::afw::image::Exposure<float, lsst::afw::image::maskPixelType> const &masterChunkExposure,
//     lsst::pex::policy::Policy &isrPolicy, 
//     lsst::pex::policy::Policy &datasetPolicy
//     );

// template
// lsst::afw::image::Exposure<double, lsst::afw::image::maskPixelType> defringeChunkExposure(
//     lsst::afw::image::Exposure<double, lsst::afw::image::maskPixelType> const &chunkExposure,   
//     lsst::afw::image::Exposure<double, lsst::afw::image::maskPixelType> const &masterChunkExposure,
//     lsst::pex::policy::Policy &isrPolicy, 
//     lsst::pex::policy::Policy &datasetPolicy
//     );
/************************************************************************/
