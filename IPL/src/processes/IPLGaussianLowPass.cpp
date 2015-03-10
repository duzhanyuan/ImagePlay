#include "IPLGaussianLowPass.h"

void IPLGaussianLowPass::init()
{
    // init
    _result     = NULL;
    _kernel     = NULL;

    // basic settings
    setClassName("IPLGaussianLowPass");
    setTitle("Gaussian Low Pass");
    setCategory(IPLProcess::CATEGORY_LOCALOPERATIONS);
    setOpenCVSupport(IPLProcess::OPENCV_OPTIONAL);
    setKeywords("blur");
    setDescription("Local Gaussian low-pass operator.");

    // inputs and outputs
    addInput("Image", IPLData::IMAGE_COLOR);
    addOutput("Image", IPLData::IMAGE_COLOR);
    addOutput("Kernel", IPLData::MATRIX);

    // properties
    addProcessPropertyDouble("sigma", "Sigma", "Standard deviation σ", IPL_DOUBLE_SLIDER, 2.0, 0.5, 25.0);
}

void IPLGaussianLowPass::destroy()
{
    delete _result;
}

bool IPLGaussianLowPass::processInputData(IPLImage* image , int, bool useOpenCV)
{
    // delete previous result
    delete _result;
    _result = NULL;
    delete _kernel;
    _kernel = NULL;

    int width  = image->width();
    int height = image->height();

    _result = new IPLImage(image->type(), width, height);

    // get properties
    double sigma = getProcessPropertyDouble("sigma");

    // cattin variant
    //int N = ceil( sigma * sqrt( 2.0*log( 1.0/0.015 ) ) + 1.0 );
    //int window = 2*N+1;


    // opencv variant
    int window = 2 * ceil((sigma - 0.8)/0.3 +1) + 1;
    int N = (window-1) / 2;

    addInformation("Window: " + std::to_string(window));
    addInformation("N: " + std::to_string(N));

    if(sigma > 10)
        addWarning("Sigma > 10 can lead to long processing time.");

    if(useOpenCV)
    {
        cv::Mat result;
        cv::GaussianBlur(image->toCvMat(), result, cv::Size(window, window), sigma);
        delete _result;
        _result = new IPLImage(result);

        return true;
    }

    float* filter = new float [window];
    float sum = 0;
    for( int k = -N; k <= N; ++k )
    {
        float val = exp( -((double)k*(double)k) / (sigma*sigma) );
        sum += val;
        filter[k+N] = val;
    }

    _kernel = new IPLMatrix(1, window, filter);

    float sumFactor = 1.0f/sum;
    for( int k = 0; k < window; ++k )
        filter[k] *= sumFactor;

    int progress = 0;
    int maxProgress = image->height() * image->getNumberOfPlanes() * 2;
    int nrOfPlanes = image->getNumberOfPlanes();

    #pragma omp parallel for
    for( int planeNr=0; planeNr < nrOfPlanes; planeNr++ )
    {
        IPLImagePlane* plane = image->plane( planeNr );
        IPLImagePlane* newplane = _result->plane( planeNr );

        /// @todo should be double image
        IPLImage* tmpI = new IPLImage( IPLData::IMAGE_GRAYSCALE, width, height );

        // horizontal run
        float sum = 0;
        float img = 0;
        int i = 0;

        for(int y=0; y<height; y++)
        {
            // progress
           notifyProgressEventHandler(100*progress++/maxProgress);

            for(int x=0; x<width; x++)
            {
                sum = 0;
                i = 0;
                for( int kx=-N; kx<=N; kx++ )
                {
                    img = plane->bp(x+kx, y);
                    sum += (img * filter[i++]);
                }
                tmpI->plane(0)->p(x,y) = sum;
            }
        }

        // vertical run
        sum = 0;
        img = 0;
        i = 0;
        for(int y=0; y<height; y++)
        {
            // progress
            notifyProgressEventHandler(100*progress++/maxProgress);

            for(int x=0; x<width; x++)
            {
                sum = 0;
                i = 0;
                for( int ky=-N; ky<=N; ky++ )
                {
                    img = tmpI->plane(0)->bp(x, y+ky);
                    sum += (img * filter[i++]);
                }
                newplane->p(x,y) = sum;
            }
        }
        delete tmpI;
    }
    delete [] filter;

    return true;
}

IPLData* IPLGaussianLowPass::getResultData(int index)
{
    if(index == 0)
        return _result;
    else
        return _kernel;
}
