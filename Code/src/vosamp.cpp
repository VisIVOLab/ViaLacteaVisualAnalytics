#include "vosamp.h"
#include "libsamp/samp.h"
#include <QDebug>


#define	opt(s)		(s?s:"INDEF")

int	sampH = 0;			/* samp struct handle	*/

static char *name	= "VisIVO";		/* metadata		*/
static char *descr	= "VisIVO - Vialactea";
char   *filt_sender	= NULL;
char   *filt_mtype	= NULL;
char   *mtype		= NULL;
extern "C"
{
static char *vos_rewriteURL (char *in);
extern char *vos_toURL (char *arg);
}

void msg_handler (char *sender, char *msg_id, int params)
{
    qDebug()<<"sender "<<sender;
    qDebug()<<"msg_id "<<msg_id;
    qDebug()<<"params "<<params;
/*
    if (filt_sender && strcasecmp (filt_sender, sender))
        return;
    if (filt_mtype && strcasecmp (filt_mtype, mtype))
        return;

    samp_printMessage (mtype, sender, msg_id, params);
*/
  }



vosamp::vosamp()
{

    sampH = sampInit (name, descr);
    /*
     * Set up some local application metadata values.  These tell the
     *  Hub and other applications who we are.
     */
    samp_Metadata (sampH, "author.email", "fabio.vitello@inaf.it");
    samp_Metadata (sampH, "author.name", "Fabio Vitello");
    samp_Metadata (sampH, "samp.icon.url", "http://visivo.oact.inaf.it/logo.png");


    /*
     *  Subscribe to all message types and install the message handler.
     */
    samp_Subscribe (sampH, "table.load.votable", (void *)msg_handler);

    /*
     * Register with the Hub and begin messaging .....
     */
    sampStartup (sampH);

    int stat=0;
    /*

    qDebug()<<"send status: "<<stat;
    stat = samp_tableLoadFITS (samp, "all",
                               "http://visivo.oact.inaf.it/thyco9999.fits",
                               "test1",
                               "thyco9999.fits");


    qDebug()<<"send status: "<<stat;
*/


    //    samp_Unsubscribe(samp, "*");

}



vosamp::~vosamp()
{

}

void vosamp::unregister()
{
    qDebug()<<" unsubscrive from samp hub";
    samp_Unsubscribe(sampH, "*");
    sampShutdown(sampH);

}

void vosamp::sendFitsImage(char* f, char* t)
{
    int  stat = samp_imageLoadFITS (sampH, t,
                                    vos_toURL(f),		/* URL/file 	*/
                                    "imgId", 		/* imgId	*/
                                    "name");		/* name		*/
}

char* vosamp::getClientsList()
{
    char *clist = NULL;

    samp_mapClients (sampH);

    clist = samp_getClients (sampH);
    return clist;

}



