#include <stdio.h>
#include <string.h>
#include "iniparser.h"
#include "utilities.h"
#include "plotcircles.h"
#include "outputfilefunctions.h"
#include "constants.h"

int PlotCircles(dictionary *scan_file_dict){
    char commandfile[400];
    const char *outputdirectory;
    int i;
    
    outputdirectory = iniparser_getstring (scan_file_dict,"settings:outputdirectory", "null");
    sprintf(commandfile,"%scirclecommands.txt",outputdirectory);
    for (i=1;i<=10;i++){
        //make a plot for each band number
        MakeCircleAndPoints(i,scan_file_dict, commandfile);  
    }
    return(0);
}


int MakeCircleAndPoints(int band, dictionary *scan_file_dict, char *commandfile){

    FILE *fileptr;
    char commandline[500];
    int i, num_scans;
    const char *gnuplot;
    char plotfilename[500];
    const char *outputdirectory;
    char linebuffer[500];
    char *title,pointtitle[100];
    char titlebuffer[500];
    float subreflector_radius = subreflector_radius12m;
    float xcenter, ycenter;
    char plotcommand[500];
    char bandkey[10];
    SCANDATA *scan_array;
    int bandfound = 0, scancount=0;
    char plotkey[10];
    char *linestype, *lineswidth;
    const char *tempsec;
    char tempseckey[200];
    char centers[10];
    char nomLegend[30];
    PointingOptionType pointingOption = NOMINAL;
    
    num_scans = GetNumberOfScans(scan_file_dict);
    scan_array = (SCANDATA *) malloc (num_scans * sizeof(SCANDATA));
    
    for(i=0;i<iniparser_getnsec(scan_file_dict);i++){
         tempsec = iniparser_getsecname(scan_file_dict,i);
         sprintf(tempseckey,"%s:scanset",tempsec);
             if (iniparser_getint (scan_file_dict, tempseckey, -1) != -1){
                     GetScanData(scan_file_dict, tempsec, &scan_array[scancount]);
                     scancount++;
             }
     }
    
    strcpy(centers, iniparser_getstring (scan_file_dict, "settings:centers", "nominal"));
    if (!strcmp(centers, "7meter")) {
        pointingOption = ACA_7METER;
        subreflector_radius = subreflector_radius7m;

    } else if(!strcmp(centers, "band1test")) {
        pointingOption = BAND1_TEST;
    }
    
    gnuplot = iniparser_getstring (scan_file_dict,"settings:gnuplot", "null");

    for(i=0;i<num_scans;i++){               
        if (scan_array[i].band == band){
              bandfound=1;                             
        }                           
    }

    if (bandfound == 1){
        remove(commandfile);
        PickNominalAngles(band, &xcenter, &ycenter, pointingOption);
        outputdirectory = iniparser_getstring (scan_file_dict,"settings:outputdirectory", "null");
        sprintf(plotfilename,"%sband%d_pointingangles.png",outputdirectory,band);
        sprintf(titlebuffer,"Band %d Pointing Angles",band);
        title = titlebuffer;
        fileptr = fopen(commandfile, "w");
        
        if(fileptr==NULL) {
           printf("Error: can't create file.\r\n");
        }
        fputs("set terminal png size 800, 800 crop\r\n",fileptr);
        //fputs("set terminal png crop\r\n",fileptr);
        sprintf(linebuffer,"set output '%s'\r\n",plotfilename);
        fputs(linebuffer,fileptr);
        sprintf(linebuffer,"set title '%s'\r\n",title);
        fputs(linebuffer,fileptr);
        
        fputs("set xlabel 'AZ (deg)'\r\n",fileptr);
        fputs("set ylabel 'EL (deg)'\r\n",fileptr);
        fputs("set parametric\r\n",fileptr);
        //fputs("set size square\r\n",fileptr);
        fputs("set key outside\r\n",fileptr);
        
        
        sprintf(plotcommand,"set xrange [%f:%f]\r\n",xcenter-(0.5*subreflector_radius)-2,xcenter+(0.5*subreflector_radius)+2);
        fputs(plotcommand,fileptr);
        sprintf(plotcommand,"set yrange [%f:%f]\r\n",ycenter-(0.5*subreflector_radius)-2,ycenter+(0.5*subreflector_radius)+2);
        fputs(plotcommand,fileptr);
        fputs("set size square\r\n",fileptr);
        sprintf(plotcommand,"plot [0:2*pi] %f+%.2f*sin(t),%f+%.2f*cos(t) title 'subreflector'",
                xcenter, subreflector_radius, ycenter, subreflector_radius);
        //fputs(plotcommand,fileptr);
        //Print nominal pointing angle
        
        int PrintNomAngle = 1;
        strcpy(nomLegend, " nominal pointing angle");
        if (!strcmp(centers, "7meter"))
            strcpy(nomLegend, " ACA 7m nominal pointing");

        for(i=0;i<num_scans;i++){  
            if ((scan_array[i].band == band) && (!strcmp(scan_array[i].type, "copol"))){  
                  //printf("scans[%d].band= %d",i,scan_array[i].band);
                  //printf("scans[%d].type= %s",i,scan_array[i].type);
                  linestype = "3";
                    lineswidth = "1";
                    if (scan_array[i].pol == 0){
                        linestype = "4";
                   } 
                   if (strcmp(scan_array[i].is4545_scan, "TRUE") != 0){
                      if (PrintNomAngle == 1){
                        sprintf(plotcommand,"%s,%f,%f with points lw 1  pt 1 title '%s'",
                        plotcommand,xcenter,ycenter,nomLegend);
                        PrintNomAngle = 0;
                      }  
                                                
                      sprintf(pointtitle,"%d GHz,pol %d,tilt %d",scan_array[i].f,scan_array[i].pol,scan_array[i].tilt);
                      sprintf(plotcommand,"%s,%f,%f with points lw %s  pt %s title '%s'",plotcommand,
                              scan_array[i].ff_xcenter,scan_array[i].ff_ycenter,lineswidth,linestype,pointtitle);  
                  }                                  
            }
                                     
        }
        fputs(plotcommand,fileptr);
        fputs("\r\n\r\n",fileptr);
        sprintf(bandkey,"band%d",band);
        sprintf(commandline,"%s %s",gnuplot,commandfile);
        fclose(fileptr); 
        
        
        //UpdateDictionary(scan_file_dict,"pointingangles", bandkey, plotfilename); 
        system(commandline);
        sprintf(plotkey,"pointingangles_band_%d",band);
        UpdateDictionary(scan_file_dict,"settings", plotkey, plotfilename);
    }//end if band found
    
    
    free (scan_array);
    
    return 1;
}
    
    

