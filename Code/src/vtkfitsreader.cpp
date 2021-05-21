#include "vtkfitsreader.h"

#include "vtkCommand.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkFloatArray.h"
#include <cmath>
#include "vtkPointData.h"
#include "qdebug.h"
#include <qmath.h>


//vtkCxxRevisionMacro(vtkFitsReader, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkFitsReader);

//----------------------------------------------------------------------------
vtkFitsReader::vtkFitsReader()
{
    this->filename[0]='\0';
    this->xStr[0]='\0';
    this->yStr[0]='\0';
    this->zStr[0]='\0';
    this->title[0]='\0';
    this->SetNumberOfInputPorts( 0 );
    this->SetNumberOfOutputPorts( 1 );
    //this->is3D=is3D;

    for (int i=0; i<3; i++)
    {
        crval[i]=0;
        cpix[i]=0;
        cdelt[i]=0;
        naxes[i]= 10;
    }
    
    this->is3D=false;

    qDebug()<<"New.vtkFitsReader";

}

//----------------------------------------------------------------------------
vtkFitsReader::~vtkFitsReader()
{
}

void vtkFitsReader::SetFileName(std::string name) {


    if (name.empty()) {
        vtkErrorMacro(<<"Null Datafile!");
        return;
    }


    filename= name;
    this->Modified();

    qDebug()<<"SetFileName.vtkFitsReader";

}
//----------------------------------------------------------------------------
void vtkFitsReader::PrintSelf(ostream& os, vtkIndent indent)
{
    // this->Superclass::PrintSelf(os, indent);
}

void vtkFitsReader::PrintHeader(ostream& os, vtkIndent indent)
{
    // this->Superclass::PrintHeader(os, indent);

}

void vtkFitsReader::PrintTrailer(std::ostream& os , vtkIndent indent)
{
    // this->Superclass::PrintTrailer(os, indent);
}

//----------------------------------------------------------------------------
vtkStructuredPoints* vtkFitsReader::GetOutput()
{
    return this->GetOutput(0);
}

//----------------------------------------------------------------------------
vtkStructuredPoints* vtkFitsReader::GetOutput(int port)
{
    return vtkStructuredPoints::SafeDownCast(this->GetOutputDataObject(port));
}

//----------------------------------------------------------------------------
void vtkFitsReader::SetOutput(vtkDataObject* d)
{
    this->GetExecutive()->SetOutputData(0, d);
}


//----------------------------------------------------------------------------
int vtkFitsReader::ProcessRequest(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector)
{
    // Create an output object of the correct type.
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    {
        return this->RequestDataObject(request, inputVector, outputVector);
    }
    // generate the data
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
        return this->RequestData(request, inputVector, outputVector);
    }

    if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
        return this->RequestUpdateExtent(request, inputVector, outputVector);
    }

    // execute information
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
        return this->RequestInformation(request, inputVector, outputVector);
    }

    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkFitsReader::FillOutputPortInformation(
        int vtkNotUsed(port), vtkInformation* info)
{
    // now add our info
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkStructuredPoints");
    return 1;
}


//----------------------------------------------------------------------------
int vtkFitsReader::RequestDataObject(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** vtkNotUsed(inputVector),
        vtkInformationVector* outputVector )
{

    qDebug()<<"RequestDataObject.vtkFitsReader";

    for ( int i = 0; i < this->GetNumberOfOutputPorts(); ++i )
    {
        
        ReadHeader();
        fitsfile *fptr;
        int status = 0, nfound = 0, anynull = 0;
        long  fpixel, nbuffer, npixels, ii;
        const int buffsize = 1000;

        float nullval, buffer[buffsize];
        vtkFloatArray *scalars = vtkFloatArray::New();

        qDebug()<<"for.RequestDataObject.vtkFitsReader";
        vtkInformation* outInfo = outputVector->GetInformationObject( i );
        vtkStructuredPoints* output = vtkStructuredPoints::SafeDownCast(
                    outInfo->Get( vtkDataObject::DATA_OBJECT() ) );
        if ( ! output )
        {
            qDebug()<<"START-!output.for.RequestDataObject.vtkFitsReader";
            output = vtkStructuredPoints::New();
            outInfo->Set( vtkDataObject::DATA_OBJECT(), output );
            output->FastDelete();

            //FITS READER CORE

            char *fn=new char[filename.length() + 1];;
            strcpy(fn, filename.c_str());

            if ( fits_open_file(&fptr, fn, READONLY, &status) )
                printerror( status );

            delete []fn;

            qDebug()<<"*******----------------- is3D "<<is3D;

            if(! (this->is3D) )
            {
                qDebug()<<"!is3D.RequestDataObject.vtkFitsReader";


                /* read the NAXIS1 and NAXIS2 keyword to get image size */
                // if ( fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nfound, &status) )
                if ( fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status) )
                    printerror( status );

                npixels  = naxes[0] * naxes[1]; /* num of pixels in the image */
                fpixel   = 1;
                nullval  = 0;                /* don't check for null values in the image */
                datamin  = 1.0E30;
                datamax  = -1.0E30;

                output->SetDimensions(naxes[0], naxes[1],1);

                // output->SetOrigin(0.0, 0.0, 0.0);
                output->SetOrigin(1.0, 1.0, 0.0);

                scalars->Allocate(npixels);

                while (npixels > 0) {

                    nbuffer = npixels;
                    if (npixels > buffsize)
                        nbuffer = buffsize;

                    if ( fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                                       buffer, &anynull, &status) )
                        printerror( status );

                    for (ii = 0; ii < nbuffer; ii++)
                    {

                        // if (isnanf(buffer[ii])) buffer[ii] = -1000000.0; // hack for now
                        if (std::isnan(buffer[ii])) buffer[ii] = -1000000.0; // hack for now

                        scalars->InsertNextValue(buffer[ii]);


                        if ( buffer[ii] < datamin  &&  buffer[ii]!=-1000000.0)
                            datamin = buffer[ii];
                        if ( buffer[ii] > datamax  &&  buffer[ii]!=-1000000.0 )
                            datamax = buffer[ii];
                    }

                    npixels -= nbuffer;    /* increment remaining number of pixels */
                    fpixel  += nbuffer;    /* next pixel to be read in image */
                }
            }
            else
            {
                this->CalculateRMS();
                qDebug()<<"Dopo calculate RMS: "<< this->GetRMS();
                qDebug()<<"datamin: "<<datamin;

                if ( fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nfound, &status) )
                    printerror( status );



                npixels  = naxes[0] * naxes[1] * naxes[2];
                npix=npixels;
                fpixel   = 1;
                nullval  = 0;
                //datamin  = 1.0E30;
                //datamax  = -1.0E30;

                QString xtrim(this->xStr);
                QString ytrim(this->yStr);
                bool swapped=false;
                if (xtrim.split("-")[0].toLower().compare("glat") ==0 && ytrim.split("-")[0].toLower().compare("glon") ==0 )
                {
                    qDebug()<<"INSIDE: "<<xtrim.split("-")[0]<<" - "<<ytrim.split("-")[0];

                    output->SetDimensions(naxes[1], naxes[0], naxes[2]);
                   // swapped=true;
                }
                else
                    output->SetDimensions(naxes[0], naxes[1], naxes[2]);
                //                output->SetOrigin(0.0, 0.0, 0.0);
                output->SetOrigin(1.0, 1.0, 1.0);


                //vtkFloatScalars *scalars = new vtkFloatScalars(npixels);
                //vtkFloatScalars *scalars = vtkFloatScalars::New();

                //vtkFloatArray *scalars = vtkFloatArray::New();
                fitsScalars= vtkFloatArray::New();
                fitsScalars->Allocate(npixels);
                scalars->Allocate(npixels);

                if (swapped)
                {

                    npixels=naxes[1];
                    fpixel=naxes[0]+1;

                    qDebug()<<"npixels "<<npixels<<" pixel "<<fpixel;
                    //For every pixel
                    while (npixels > 0) {

                        nbuffer = npixels;
                        if (npixels > buffsize)
                            nbuffer = buffsize;


                        if ( fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                                           buffer, &anynull, &status) )
                            printerror( status );
                        for (ii = 0; ii < nbuffer; ii++)
                        {

                            if (std::isnan(buffer[ii]))
                            {


                                buffer[ii] = -1000000.0; // hack for now
                            }
                            //conversion
                            //CVAL3 + (X - CPIX3)*CDEL3

                            buffer[ii]=crval[2]/1000+(buffer[ii]-cpix[2])*cdelt[2]/1000;



                            scalars->InsertNextValue(buffer[ii]);
                            fitsScalars->InsertNextValue(buffer[ii]);

                        }

                        npixels -= nbuffer;
                        fpixel  += nbuffer;
                    }


                    npixels=naxes[0];
                    fpixel=1;
                    qDebug()<<"npixels "<<npixels<<" pixel "<<fpixel;

                    //For every pixel
                    while (npixels > 0) {

                        nbuffer = npixels;
                        if (npixels > buffsize)
                            nbuffer = buffsize;


                        if ( fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                                           buffer, &anynull, &status) )
                            printerror( status );
                        for (ii = 0; ii < nbuffer; ii++)
                        {

                            if (std::isnan(buffer[ii]))
                            {


                                buffer[ii] = -1000000.0; // hack for now
                            }
                            //conversion
                            //CVAL3 + (X - CPIX3)*CDEL3

                            buffer[ii]=crval[2]/1000+(buffer[ii]-cpix[2])*cdelt[2]/1000;



                            scalars->InsertNextValue(buffer[ii]);
                            fitsScalars->InsertNextValue(buffer[ii]);

                        }

                        npixels -= nbuffer;
                        fpixel  += nbuffer;
                    }


                    npixels=naxes[2];
                    fpixel=naxes[0]+naxes[1]+1;
                    qDebug()<<"npixels "<<npixels<<" pixel "<<fpixel;

                    //For every pixel
                    while (npixels > 0) {

                        nbuffer = npixels;
                        if (npixels > buffsize)
                            nbuffer = buffsize;


                        if ( fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                                           buffer, &anynull, &status) )
                            printerror( status );
                        for (ii = 0; ii < nbuffer; ii++)
                        {

                            if (std::isnan(buffer[ii]))
                            {


                                buffer[ii] = -1000000.0; // hack for now
                            }
                            //conversion
                            //CVAL3 + (X - CPIX3)*CDEL3

                            buffer[ii]=crval[2]/1000+(buffer[ii]-cpix[2])*cdelt[2]/1000;



                            scalars->InsertNextValue(buffer[ii]);
                            fitsScalars->InsertNextValue(buffer[ii]);

                        }

                        npixels -= nbuffer;
                        fpixel  += nbuffer;
                    }


                }
                else
                {
                    //For every pixel
                    while (npixels > 0) {

                        nbuffer = npixels;
                        if (npixels > buffsize)
                            nbuffer = buffsize;


                        if ( fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                                           buffer, &anynull, &status) )
                            printerror( status );
                        float tmp;
                        int index;
                        for (ii = 0; ii < nbuffer; ii++)
                        {

                            //if (isnanf(buffer[ii])) buffer[ii] = -1000000.0; // hack for now


                            if (std::isnan(buffer[ii]))
                            {


                                buffer[ii] = -1000000.0; // hack for now
                            }
                            //conversion
                            //CVAL3 + (X - CPIX3)*CDEL3

                            buffer[ii]=crval[2]/1000+(buffer[ii]-cpix[2])*cdelt[2]/1000;



                            scalars->InsertNextValue(buffer[ii]);
                            fitsScalars->InsertNextValue(buffer[ii]);

                            //  qDebug()<<"buffer["<<ii<<"]"<<buffer[ii];
                            //  if ( buffer[ii] < datamin )
                            //  datamin = buffer[ii];
                            //  if ( buffer[ii] > datamax )
                            //  datamax = buffer[ii];
                        }

                        npixels -= nbuffer;
                        fpixel  += nbuffer;
                    }
                } //end else
            }
            
        }


        // cerr << "min: " << datamin << " max: " << datamax << endl;

        if ( fits_close_file(fptr, &status) )
            printerror( status );

        output->GetPointData()->SetScalars(scalars);

        //END FITS READ CORE
        this->GetOutputPortInformation( i )->Set(vtkDataObject::DATA_EXTENT_TYPE(), output->GetExtentType() );

        qDebug()<<"END-!output.for.RequestDataObject.vtkFitsReader";

    }

    return 1;
}

//----------------------------------------------------------------------------
int vtkFitsReader::RequestInformation(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** vtkNotUsed(inputVector),
        vtkInformationVector* vtkNotUsed(outputVector))
{
    // do nothing let subclasses handle it
    return 1;
}

//----------------------------------------------------------------------------
int vtkFitsReader::RequestUpdateExtent(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* vtkNotUsed(outputVector))
{
    qDebug()<<" \t \t **RequestUpdateExtent.vtkFitsReader";
    int numInputPorts = this->GetNumberOfInputPorts();
    for (int i=0; i<numInputPorts; i++)
    {
        int numInputConnections = this->GetNumberOfInputConnections(i);
        for (int j=0; j<numInputConnections; j++)
        {
            vtkInformation* inputInfo = inputVector[i]->GetInformationObject(j);
            inputInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
        }
    }
    return 1;
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an imaging style Execute method.
int vtkFitsReader::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** vtkNotUsed( inputVector ),
        vtkInformationVector* vtkNotUsed(outputVector) )
{
    qDebug()<<"\t\t *** RequestData.vtkFitsReader";

    // do nothing let subclasses handle it
    return 1;
}

void vtkFitsReader::ReadHeader() {



    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */

    int status, nkeys, keypos, hdutype, ii, jj;
    char card[FLEN_CARD];   /* standard string lengths defined in fitsioc.h */
    
    
    char crval1[80];
    char crval2[80];
    char crval3[80];
    char crpix1[80];
    char crpix2[80];
    char crpix3[80];
    char cdelt1[80];
    char cdelt2[80];
    char cdelt3[80];
    char naxis1[80];
    char naxis2[80];
    char naxis3[80];
    
    
    crval1[0] ='\0';
    crval2[0] ='\0';
    crval3[0] ='\0';
    crpix1[0] ='\0';
    crpix2[0] ='\0';
    crpix3[0] ='\0';
    cdelt1[0] ='\0';
    cdelt2[0] ='\0';
    cdelt3[0] ='\0';
    
    QString val1, val2, val3, pix1,pix2, pix3, delt1, delt2, delt3, nax1, nax2, nax3;

    status = 0;


    char *fn=new char[filename.length() + 1];;
    strcpy(fn, filename.c_str());

    if ( fits_open_file(&fptr, fn, READONLY, &status) )
        printerror( status );
    delete []fn;

    /* attempt to move to next HDU, until we get an EOF error */
    for (ii = 1; !(fits_movabs_hdu(fptr, ii, &hdutype, &status) ); ii++)
    {

        /* get no. of keywords */
        if (fits_get_hdrpos(fptr, &nkeys, &keypos, &status) )
            printerror( status );

        for (jj = 1; jj <= nkeys; jj++)  {

            if ( fits_read_record(fptr, jj, card, &status) )
                printerror( status );

            if (!strncmp(card, "CTYPE", 5)) {
                // cerr << card << endl;
                char *first = strchr(card, '\'');
                char *last = strrchr(card, '\'');

                *last = '\0';
                if (card[5] == '1')
                    strcpy(xStr, first+1);
                if (card[5] == '2')
                    strcpy(yStr, first+1);
                if (card[5] == '3')
                    strcpy(zStr, first+1);

                qDebug()<<"xStr: "<<xStr;
                qDebug()<<"yStr: "<<yStr;
            }

            if (!strncmp(card, "OBJECT", 6)) {
                cerr << card << endl;
                char *first = strchr(card, '\'');
                char *last = strrchr(card, '\'');
                *last = '\0';
                strcpy(title, first+1);
            }

            if (!strncmp(card, "CRVAL", 5)) {
                char *first = strchr(card, '=');
                char *last = strrchr(card, '=');
                *last = '\0';

                // char *last = strrchr(card, '/');
                //*last = '\0';

                if (card[5] == '1')
                {
                    strcpy(crval1, first+1);
                    char *pch = strtok (crval1," ,");
                    strcpy(crval1, pch);
                    
                }
                
                if (card[5] == '2')
                {
                    strcpy(crval2, first+1);
                    char *pch = strtok (crval2," ,");
                    strcpy(crval2, pch);

                }
                
                if (card[5] == '3')
                {
                    strcpy(crval3, first+1);
                    char *pch = strtok (crval3," ,");
                    strcpy(crval3, pch);

                }
            }

            if (!strncmp(card, "CRPIX", 5)) {
                char *first = strchr(card, '=');
                char *last = strrchr(card, '=');
                *last = '\0';
                
                
                if (card[5] == '1')
                {
                    strcpy(crpix1, first+1);

                    char *pch = strtok (crpix1," ,");
                    strcpy(crpix1, pch);
                }
                
                if (card[5] == '2')
                {
                    strcpy(crpix2, first+1);

                    char *pch = strtok (crpix2," ,");
                    strcpy(crpix2, pch);
                }
                if (card[5] == '3')
                {
                    strcpy(crpix3, first+1);

                    char *pch = strtok (crpix3," ,");
                    strcpy(crpix3, pch);
                }
            }

            if (!strncmp(card, "CDELT", 5)) {
                char *first = strchr(card, '=');
                char *last = strrchr(card, '=');
                *last = '\0';
                
                if (card[5] == '1')
                {
                    strcpy(cdelt1, first+1);
                    char *pch = strtok (cdelt1," ,");
                    strcpy(cdelt1, pch);
                    
                }
                
                if (card[5] == '2')
                {
                    strcpy(cdelt2, first+1);
                    char *pch = strtok (cdelt2," ,");
                    strcpy(cdelt2, pch);
                }
                
                if (card[5] == '3')
                {
                    strcpy(cdelt3, first+1);
                    char *pch = strtok (cdelt3," ,");
                    strcpy(cdelt3, pch);
                }
            }
            
            

        }
    }


    val1=crval1;
    val2=crval2;
    val3=crval3;
    pix1=crpix1;
    pix2=crpix2;
    pix3=crpix3;
    delt1=cdelt1;
    delt2=cdelt2;
    delt3=cdelt3;


    
    crval[0]=val1.toDouble(); //problema
    crval[1]=val2.toDouble();
    crval[2]=val3.toDouble();
    cpix[0]=pix1.toDouble();
    cpix[1]=pix2.toDouble();
    cpix[2]=pix3.toDouble();
    cdelt[0]=delt1.toDouble();
    cdelt[1]=delt2.toDouble();
    cdelt[2]=delt3.toDouble();

    initSlice=crval[2]-(cdelt[2]*(cpix[2]-1));
    
    

    

    
}

// Note: from cookbook.c in fitsio distribution.
void vtkFitsReader::printerror(int status) {

    cerr << "vtkFitsReader ERROR.";
    if (status) {
        fits_report_error(stderr, status); /* print error report */
        exit( status );    /* terminate the program, returning error status */
    }
    return;
}

void vtkFitsReader::CalculateMoment()
{
    ReadHeader();

    fitsfile *fptr;
    int status = 0, nfound = 0;

    if (fits_open_file(&fptr, filename.c_str(), READONLY, &status))
        printerror(status);
    if (fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nfound, &status))
        printerror(status);

    long npixels = naxes[0] * naxes[1] * naxes[2];
    long buffsize = naxes[0] * naxes[1];
    float *buffer = new float[buffsize];
    auto scalars = vtkFloatArray::New();
    scalars->Allocate(buffsize);

    for (long i = 0; i < buffsize; ++i)
        scalars->InsertNextValue(0.0);

    int anynull = 0;
    float fpixel = 1, nullval = 0;
    long nbuffer = 0;

    while (npixels > 0) {
        nbuffer = fmin(npixels, buffsize);

        if (fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval, buffer, &anynull, &status))
                printerror(status);

        for (long i = 0; i < nbuffer; ++i) {
            if (std::isnan(buffer[i]))
                buffer[i] = -1000000.0;

            float v = scalars->GetValue(i) + buffer[i];
            scalars->SetValue(i, v);
        }

        fpixel += nbuffer;
        npixels -= nbuffer;
    }

    double range[2];
    scalars->GetRange(range);


    delete [] buffer;

    if (fits_close_file(fptr, &status))
        printerror(status);

    auto output = GetOutput();
    output->SetDimensions(naxes[0], naxes[1], 1);
    output->GetPointData()->SetScalars(scalars);
}

// Note: This function adapted from readimage() from cookbook.c in
// fitsio distribution.
void vtkFitsReader::CalculateRMS() {
    

    ReadHeader();
    
    vtkStructuredPoints *output = (vtkStructuredPoints *) this->GetOutput();
    fitsfile *fptr;
    int status = 0, nfound = 0, anynull = 0;
    long fpixel, nbuffer, npixels, ii, n=0;
    double meansquare=0;
    const int buffsize = 1000;
    
    
    float nullval, buffer[buffsize];
    char *fn=new char[filename.length() + 1];
    strcpy(fn, filename.c_str());
    
    if ( fits_open_file(&fptr, fn, READONLY, &status) )
        printerror( status );
    
    delete []fn;
    vtkFloatArray *scalars = vtkFloatArray::New();
    if ( fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nfound, &status) )
        printerror( status );
    
    npixels  = naxes[0] * naxes[1] * naxes[2];
    n=npixels;
    
    fpixel   = 1;
    nullval  = 0;
    datamin  = 1.0E30;
    datamax  = -1.0E30;
    /*
    cerr << "\nvtkFitsReader: calculating the RMS" << this->filename << endl;
    cerr << "Dim: " << naxes[0] << " " << naxes[1] << " " << naxes[2] << endl;
    cerr << "points: " << npixels << endl;
    cerr << "creating vtk structured points dataset" << endl;
   */
    output->SetDimensions(naxes[0], naxes[1], naxes[2]);
    output->SetOrigin(0.0, 0.0, 0.0);
    
    scalars->Allocate(npixels);
    int bad=0;
    int slice;
    int num=0;



    minmaxslice=new float*[naxes[2]];
    for(int i=0;i< naxes[2];i++)
    {

        minmaxslice[i] = new float[2];

        minmaxslice[i][0]= 1.0E30;
        minmaxslice[i][1]= -1.0E30;

    }

    //For every pixel
    while (npixels > 0) {




        nbuffer = npixels;
        if (npixels > buffsize)
            nbuffer = buffsize;
        
        if ( fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                           buffer, &anynull, &status) )
            printerror( status );


        for (ii = 0; ii < nbuffer; ii++)  {
            // slice= (num/(naxes[0]*naxes[1]))%(naxes[0]*naxes[1]);
            slice= (num/ (naxes[0]*naxes[1]) );
            num++;

            // qDebug()<<"npixel: "<<num <<" è sulla slice "<< slice <<" x: "<<naxes[0]<<" y: "<<naxes[1]<<" z: "<<naxes[2];

            if (std::isnan(buffer[ii]))
                buffer[ii] = -1000000.0;
            scalars->InsertNextValue(buffer[ii]);

            if ( buffer[ii]!=-1000000.0)
            {
                if ( buffer[ii] < datamin )
                    datamin = buffer[ii];
                if ( buffer[ii] > datamax   )
                    datamax = buffer[ii];

                //qDebug()<<"poreeeee "<<slice;
                if ( buffer[ii] < minmaxslice[slice][0] )
                    minmaxslice[slice][0] = buffer[ii];
                if ( buffer[ii] > minmaxslice[slice][1]   )
                    minmaxslice[slice][1] = buffer[ii];

                //meansquare+=buffer[ii]*buffer[ii];
                //  media+=buffer[ii];
                meansquare+=buffer[ii]*buffer[ii];

            }
            else
                bad++;
        }
        
        npixels -= nbuffer;
        fpixel  += nbuffer;
    }
    /*
    media=media/n;
    float diff;
    for(ii=0; ii<n; ii++)
    {
        if (scalars->GetValue(ii)!=-1000000.0)
        {
            meansquare+=scalars->GetValue(ii)*scalars->GetValue(ii);
            diff=scalars->GetValue(ii)-media;
            sigma+=qPow(diff, 2);
        }
        else
            bad++;
    }
    
*/
    n=n-bad;
    double means=meansquare/n;
    rms=qSqrt(means);
    // sigma=qSqrt(sigma/n);
    qDebug()<<"rms: "<<rms<<" badpixel: "<<bad<<" x: "<<naxes[0]<<" y: "<<naxes[1]<<" z: "<<naxes[2]<<" Npixels: "<< naxes[0] * naxes[1] * naxes[2];

    if ( fits_close_file(fptr, &status) )
        printerror( status );
    
    output->GetPointData()->SetScalars(scalars);
    return;
}
int vtkFitsReader::GetNaxes(int i)
{

    return naxes[i];

}
float* vtkFitsReader::GetRangeSlice(int i)
{

    return minmaxslice[i];

}
