/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************\ 
 **                                                           (C)1994 RUS  **
 **                                                                        **
 ** Description:  COVISE ShowUsg application module                       **
 **                                                                        **
 **                                                                        **
 **                             (C) 1995                                   **
 **                Computer Center University of Stuttgart                 **
 **                            Allmandring 30                              **
 **                            70550 Stuttgart                             **
 **                                                                        **
 **                                                                        **
 ** Author:  Uwe Woessner                                                  **
 **                                                                        **
 **                                                                        **
 ** Date:  27.01.95  V1.0                                                  **
\**************************************************************************/

#include <appl/ApplInterface.h>
#include "ShowUsg.h"

//
// static stub callback functions calling the real class
// member functions
//

void Application::quitCallback(void *userData, void *callbackData)
{
    Application *thisApp = (Application *)userData;
    thisApp->quit(callbackData);
}

void Application::computeCallback(void *userData, void *callbackData)
{
    Application *thisApp = (Application *)userData;
    thisApp->compute(callbackData);
}

//
//
//..........................................................................
//
//

//======================================================================
// Called before module exits
//======================================================================
void Application::quit(void *)
{
    //
    // ...... delete your data here .....
    //
    Covise::log_message(__LINE__, __FILE__, "Quitting now");
}

const char *gtype = NULL;
char *ColorIn = NULL, *colorn, *type;
float *x_in, *y_in, *z_in, *x_ina, *y_ina, *z_ina;
float *r_in, *g_in, *b_in;
int idim, jdim, kdim, option, num_set_elem, c_num_set_elem, colorSize;
int t_num_set_elem, binding = 1;
int *cl, *el, *tl, numelem, numconn, numcoord, pos;
coDoLines *Lines = NULL;
coDoPoints *Points = NULL;
coDoTriangleStrips *Strips = NULL;
coDoGeometry *Geometry = NULL;
coDoSet *Geometry_set = NULL, *Color_set_set = NULL;
coDoSet *Geometry_set_set = NULL;
coDoSet *Geometry_set_set_set = NULL;
coDoSet *color_set_2 = NULL;
int Geometrys_num = 0;
coDistributedObject **Geometrys = NULL;
coDoSet **Geometrys_set = NULL, **Colors_set_set = NULL;
coDoSet **Geometrys_set_set = NULL;
coDoSet **Geometrys_set_set_set = NULL;
coDistributedObject **color_sets_2 = NULL;
coDoSet **Colors_set = NULL;
int color_sets_2_num = 0;
coDoVec3 *u_colors;
coDoRGBA *p_colors;
// coDoVec3 *u_colors_s,*u_colors_l,*u_colors_p;
coDoRGBA *p_colors_s, *p_colors_l, *p_colors_p;
FILE *fp;
char *cname = NULL;
char *isflat = NULL;
char *min_max;
int *p_col, gennormals;

//======================================================================
// Computation routine (called when START message arrrives)
//======================================================================
void Application::compute(void *)
{
    coDistributedObject *data_obj, *color_obj, *tmp_obj, *tmp_obj2, *tmp_obj3 = NULL, *const * data_objs, *const * color_objs;
    coDistributedObject **t_data_objs, **t_color_objs;
    coDoSet *data_set;
    coDoSet *color_set;
    coDoUnstructuredGrid *uns_grid_in;
    //DO_Char_Array*		GroupArray=NULL;
    int i, u, n, num_elem = 0, pmin, pmax, istimesteps;
    char *GridIn, *GroupIn, *GeometryN;
    char buf[400];
    char *group = NULL;
    p_col = NULL;
    //	get input data object names
    GridIn = Covise::get_object_name("meshIn");
    ColorIn = Covise::get_object_name("colors");
    GroupIn = Covise::get_object_name("group");

    //	get output data object	names
    GeometryN = Covise::get_object_name("geometry");

    //	get parameter
    Covise::get_choice_param("options", &option);
    Covise::get_slider_param("pos", &pmin, &pmax, &pos);
    Covise::get_boolean_param("gennormals", &gennormals);

    // open ascii file for color names
    fp = Covise::fopen("share/covise/rgb.txt", "r");
    if (fp == NULL)
    {
        Covise::sendWarning("WARNING: Cannot open rgb text file for colored names");
    }

    //	retrieve grid object from shared memeory
    if (GridIn == NULL)
    {
        Covise::sendError("ERROR: Object name not correct for 'meshIn'");
        return;
    }
    tmp_obj = new coDistributedObject(GridIn);
    data_obj = tmp_obj->createUnknown();
    num_set_elem = 0;
    if (data_obj != 0L)
    {
        gtype = data_obj->getType();
        if (strcmp(gtype, "UNSGRD") == NULL)
        {
            uns_grid_in = (coDoUnstructuredGrid *)data_obj;
            uns_grid_in->getGridSize(&numelem, &numconn, &numcoord);
            uns_grid_in->getAddresses(&el, &cl, &x_in, &y_in, &z_in);
            uns_grid_in->getTypeList(&tl);
            cname = uns_grid_in->getAttribute("COLOR");
            isflat = uns_grid_in->getAttribute("FLAT");
        }
        else if (strcmp(gtype, "SETELE") == NULL)
        {
            data_set = (coDoSet *)data_obj;
            data_objs = data_set->getAllElements(&num_set_elem);
            if ((min_max = data_set->getAttribute("TIMESTEP")) == NULL)
                istimesteps = 0;
            else
                istimesteps = 1;
            if (num_set_elem == 0)
            {
                Covise::sendError("ERROR: Set is empty");
                return;
            }
        }

        else
        {
            Covise::sendError("ERROR: Data object 'meshIn' has wrong data type");
            return;
        }
    }
    else
    {
        Covise::sendError("ERROR: Data object 'meshIn' can't be accessed in shared memory");
        return;
    }
    if (ColorIn != 0L)
    {
        tmp_obj3 = new coDistributedObject(ColorIn);
        color_obj = tmp_obj3->createUnknown();
        if (color_obj != 0L)
        {
            type = color_obj->getType();
            if (strcmp(type, "USTVDT") == NULL)
            {
                u_colors = (coDoVec3 *)color_obj;
                u_colors->getAddresses(&r_in, &g_in, &b_in);
                colorSize = u_colors->getNumPoints();
                if (colorSize == 0)
                    Covise::sendWarning("WARNING: Data object 'colors' is empty");
            }

            else if (strcmp(type, "RGBADT") == NULL)
            {
                p_colors = (coDoRGBA *)color_obj;
                colorSize = p_colors->getNumElements();
                p_colors->getAddress(&p_col);
                if (colorSize == 0)
                {
                    Covise::sendWarning("WARNING: Data object 'colors' is empty");
                }
            }

            else if (strcmp(type, "SETELE") == NULL)
            {
                color_set = (coDoSet *)color_obj;
                color_objs = color_set->getAllElements(&c_num_set_elem);
                if (c_num_set_elem != num_set_elem)
                {
                    Covise::sendError("ERROR: Color Set has wrong number of elements");
                    return;
                }
            }
        }
        else
        {
            strcpy(buf, "WARNING: Data object 'colors' can't be accessed in shared memory");
            strcat(buf, "Create geometry object without colors. ");
            Covise::sendWarning(buf);
            ColorIn = 0L;
        }
    }
    if (GroupIn != NULL)
    {
        tmp_obj2 = new coDistributedObject(GroupIn);
        data_obj = tmp_obj->createUnknown();
        if (data_obj != 0L)
        {
            gtype = data_obj->getType();
            if (strcmp(gtype, "CARRAY") == NULL)
            {
                //		    GroupArray = (DO_Char_Array *)data_obj;
                //		    num_elem=GroupArray->getNumPoints();
                //		    GroupArray->getAddress(&group);
            }
            else if (strcmp(gtype, "SETELE") == NULL)
            {
                Covise::sendError("ERROR: Sorry, Sets are not jet supported");
                return;
            }
            else
            {
                Covise::sendError("ERROR: Data object 'group' has wrong data type");
                return;
            }
        }
        else
        {
            Covise::sendError("ERROR: Data object 'group' can't be accessed in shared memory");
            return;
        }
        if (GeometryN == NULL)
        {
            Covise::sendError("ERROR: Object name not correct for 'geometry'");
            return;
        }
        if (num_elem != numelem)
        {
            Covise::sendError("ERROR: Size of unstructured grid does not match array");
            return;
        }
    }
    if (num_set_elem == 0)
    {
        if (ColorIn != NULL)
        {
            if (colorSize == numcoord)
            {
                binding = 1;
            }
            else if (colorSize == numelem)
            {
                binding = 2;
            }
            else
            {
                Covise::sendError("Wrong number of colors");
            }
        }
        if (gennormals)
            normal_sets_2 = new coDistributedObject *[4];
        color_sets_2 = new coDistributedObject *[4];
        color_sets_2_num = 0;
        sprintf(buf, "%s_s", GeometryN);
        genpolygons(buf);
        color_sets_2[color_sets_2_num] = NULL;
        strcpy(buf, GeometryN);
        strcat(buf, "_c2");
        color_set_2 = new coDoSet(buf, color_sets_2);
        if (gennormals)
        {
            strcpy(buf, GeometryN);
            strcat(buf, "_n2");
            normal_set_2 = new coDoSet(buf, normal_sets_2);
        }
        for (i = 0; i < color_sets_2_num; i++)
            delete color_sets_2[i];
        delete[] color_sets_2;
        Geometry = new coDoGeometry(GeometryN, Geometry_set);
        if (ColorIn != NULL)
        {
            if (binding == 1)
                Geometry->setColor(PER_VERTEX, color_set_2);
            else
                Geometry->setColor(PER_FACE, color_set_2);
        }
        if (!Geometry->objectOk())
        {
            Covise::sendError("ERROR: creation of data object 'geometry' failed");
            return;
        }
        delete uns_grid_in;
        if (group != NULL)
        {
            //		delete GroupArray;
            delete color_set;
            delete tmp_obj2;
        }
        delete color_set_2;
        delete Geometry_set;
        delete Geometry;
    }
    else
    {
        if (istimesteps)
        {
            t_data_objs = data_objs;
            t_color_objs = color_objs;
            t_num_set_elem = num_set_elem;
            Geometrys_set_set_set = new coDoSet *[t_num_set_elem + 1];
            Colors_set_set = new coDoSet *[t_num_set_elem + 1];
            for (n = 0; n < t_num_set_elem; n++)
            {
                sprintf(buf, "Current timestep =  %d \n", n);
                Covise::sendWarning(buf);
                data_set = (coDoSet *)t_data_objs[n];
                data_objs = data_set->getAllElements(&num_set_elem);
                color_objs = NULL;
                if (ColorIn != NULL)
                {
                    data_set = (coDoSet *)t_color_objs[n];
                    color_objs = data_set->getAllElements(&c_num_set_elem);
                }
                Geometrys_set_set = new coDoSet *[num_set_elem + 1];
                Colors_set = new coDoSet *[num_set_elem + 1];
                for (i = 0; i < num_set_elem; i++)
                {
                    Points = NULL;
                    Lines = NULL;
                    Strips = NULL;
                    uns_grid_in = (coDoUnstructuredGrid *)data_objs[i];
                    uns_grid_in->getGridSize(&numelem, &numconn, &numcoord);
                    uns_grid_in->getAddresses(&el, &cl, &x_in, &y_in, &z_in);
                    uns_grid_in->getTypeList(&tl);
                    if (gennormals)
                    {
                        normal_sets_2 = new coDistributedObject *[4];
                        normal_sets_2_num = 0;
                    }
                    color_sets_2 = new coDistributedObject *[4];
                    color_sets_2_num = 0;
                    cname = uns_grid_in->getAttribute("COLOR");
                    isflat = uns_grid_in->getAttribute("FLAT");
                    if (ColorIn != NULL)
                    {
                        type = color_objs[i]->getType();
                        if (strcmp(type, "USTVDT") == NULL)
                        {
                            u_colors = (coDoVec3 *)color_objs[i];
                            u_colors->getAddresses(&r_in, &g_in, &b_in);
                            colorSize = u_colors->getNumPoints();
                            if (colorSize == 0)
                                Covise::sendWarning("WARNING: Data object 'colors' is empty");
                        }

                        else if (strcmp(type, "RGBADT") == NULL)
                        {
                            p_colors = (coDoRGBA *)color_objs[i];
                            colorSize = p_colors->getNumElements();
                            p_colors->getAddress(&p_col);
                            if (colorSize == 0)
                            {
                                Covise::sendWarning("WARNING: Data object 'colors' is empty");
                            }
                        }

                        if (colorSize == numcoord)
                        {
                            binding = 1;
                        }
                        else if (colorSize == numelem)
                        {
                            binding = 2;
                        }
                        else
                            Covise::sendError("Wrong number of colors");
                    }
                    sprintf(buf, "%s_ts%d_%d", GeometryN, n, i);
                    genpolygons(buf);
                    if (gennormals)
                    {
                        normal_sets_2[normal_sets_2_num] = NULL;
                        strcpy(buf, GeometryN);
                        strcat(buf, "_n2");
                        normal_set_2 = new coDoSet(buf, normal_sets_2);
                        color_sets_2[color_sets_2_num] = NULL;
                        sprintf(buf, "%s_ts%d_%dc2", GeometryN, n, i);
                        color_set_2 = new coDoSet(buf, color_sets_2);
                        for (u = 0; u < color_sets_2_num; u++)
                            delete color_sets_2[u];
                        delete[] color_sets_2;
                    }
                    color_sets_2[color_sets_2_num] = NULL;
                    sprintf(buf, "%s_ts%d_%dc2", GeometryN, n, i);
                    color_set_2 = new coDoSet(buf, color_sets_2);
                    for (u = 0; u < color_sets_2_num; u++)
                        delete color_sets_2[u];
                    delete[] color_sets_2;
                    if (ColorIn != NULL)
                        delete color_objs[i];
                    // delete color_set_2;
                    delete uns_grid_in;
                    //if(Points)
                    //delete Points;
                    //if(Lines)
                    //delete Lines;
                    //if(Strips)
                    //delete Strips;
                    // delete Geometry_set;
                    Geometrys_set_set[i] = Geometry_set;
                    Colors_set[i] = color_set_2;
                }
                Geometrys_set_set[num_set_elem] = NULL;
                Colors_set[num_set_elem] = NULL;
                sprintf(buf, "%s_ts%d", GeometryN, n);
                Geometry_set_set = new coDoSet(buf, (coDistributedObject **)Geometrys_set_set);
                sprintf(buf, "%s_ts%d_c", GeometryN, n);
                color_set = new coDoSet(buf, (coDistributedObject **)Colors_set);
                for (i = 0; i < num_set_elem; i++)
                {
                    delete Geometrys_set_set[i];
                    delete Colors_set[i];
                }
                delete[] Geometrys_set_set;
                delete[] Colors_set;
                Geometrys_set_set_set[n] = Geometry_set_set;
                Colors_set_set[n] = color_set;
                // delete Geometry_set_set;
                // delete color_set;
            }
            strcpy(buf, GeometryN);
            strcat(buf, "_ts");
            Geometrys_set_set_set[t_num_set_elem] = NULL;
            Colors_set_set[t_num_set_elem] = NULL;
            Geometry_set_set_set = new coDoSet(buf, (coDistributedObject **)Geometrys_set_set_set);
            Geometry_set_set_set->addAttribute("TIMESTEP", min_max);
            strcpy(buf, GeometryN);
            strcat(buf, "_ts_c");
            Color_set_set = new coDoSet(buf, (coDistributedObject **)Colors_set_set);
            for (i = 0; i < t_num_set_elem; i++)
            {
                delete Geometrys_set_set_set[i];
                delete Colors_set_set[i];
            }
            delete[] Geometrys_set_set_set;
            delete[] Colors_set_set;
            Geometry = new coDoGeometry(GeometryN, Geometry_set_set_set);
            if (Geometry->objectOk())
            {
                //  if(c_num_set_elem>0)
                //  {
                Geometry->setColor(PER_VERTEX, Color_set_set);
                delete Color_set_set;
                // }
                //  else
                //  delete Color_set_set; // WICHTIG!!! ganz loeschen nicht vergessen
            }
            else
            {
                Covise::sendError("ERROR: creation of data object 'geometry' failed");
                return;
            }
            delete Geometry;
        }
        else
        {
            Geometrys_set_set = new coDoSet *[num_set_elem + 1];
            Colors_set = new coDoSet *[num_set_elem + 1];
            for (i = 0; i < num_set_elem; i++)
            {
                uns_grid_in = (coDoUnstructuredGrid *)data_objs[i];
                uns_grid_in->getGridSize(&numelem, &numconn, &numcoord);
                uns_grid_in->getAddresses(&el, &cl, &x_in, &y_in, &z_in);
                uns_grid_in->getTypeList(&tl);
                cname = uns_grid_in->getAttribute("COLOR");
                isflat = uns_grid_in->getAttribute("FLAT");
                sprintf(buf, "%s_%dc2", GeometryN, i);
                color_sets_2 = new coDistributedObject *[4];
                color_sets_2_num = 0;
                //color_set->addElement(color_set_2);
                if (ColorIn != NULL)
                {
                    type = color_objs[i]->getType();
                    if (strcmp(type, "USTVDT") == NULL)
                    {
                        u_colors = (coDoVec3 *)color_objs[i];
                        u_colors->getAddresses(&r_in, &g_in, &b_in);
                        colorSize = u_colors->getNumPoints();
                        if (colorSize == 0)
                            Covise::sendWarning("WARNING: Data object 'colors' is empty");
                    }
                    else if (strcmp(type, "RGBADT") == NULL)
                    {
                        p_colors = (coDoRGBA *)color_objs[i];
                        colorSize = p_colors->getNumElements();
                        p_colors->getAddress(&p_col);
                        if (colorSize == 0)
                        {
                            Covise::sendWarning("WARNING: Data object 'colors' is empty");
                        }
                    }
                    if (colorSize == numcoord)
                    {
                        binding = 1;
                    }
                    if (colorSize == numelem)
                    {
                        binding = 2;
                    }
                    else
                        Covise::sendError("Wrong number of colors");
                }
                sprintf(buf, "%s_%d", GeometryN, i);
                genpolygons(buf);
                color_sets_2[color_sets_2_num] = NULL;
                sprintf(buf, "%s_ts%d_%dc2", GeometryN, n, i);
                color_set_2 = new coDoSet(buf, color_sets_2);
                for (u = 0; u < color_sets_2_num; u++)
                    delete color_sets_2[u];
                delete[] color_sets_2;
                if (ColorIn != NULL)
                    delete color_objs[i];
                // delete color_set_2;
                delete uns_grid_in;
                // if(Points)
                // delete Points;
                // if(Lines)
                // delete Lines;
                // if(Strips)
                // delete Strips;
                //Geometry_set_set->addElement(Geometry_set);
                //delete Geometry_set;
                Geometrys_set_set[i] = Geometry_set;
                Colors_set[i] = color_set_2;
            }
            Geometrys_set_set[num_set_elem] = NULL;
            Colors_set[num_set_elem] = NULL;
            sprintf(buf, "%s_ts%d", GeometryN, n);
            Geometry_set_set = new coDoSet(buf, (coDistributedObject **)Geometrys_set_set);
            sprintf(buf, "%s_ts%d_c", GeometryN, n);
            color_set = new coDoSet(buf, (coDistributedObject **)Colors_set);
            for (i = 0; i < num_set_elem; i++)
            {
                delete Geometrys_set_set[i];
                delete Colors_set[i];
            }
            delete[] Geometrys_set_set;
            delete[] Colors_set;

            Geometry = new coDoGeometry(GeometryN, Geometry_set_set);
            if (Geometry->objectOk())
            {
                if (c_num_set_elem > 0)
                {
                    Geometry->setColor(PER_VERTEX, color_set);
                    delete color_set;
                }
                else
                    delete color_set; // WICHTIG!!! ganz loeschen nicht vergessen
            }
            else
            {
                Covise::sendError("ERROR: creation of data object 'geometry' failed");
                return;
            }
            delete Geometry_set_set;
            delete Geometry;
        }
    }
    delete tmp_obj;
    if (tmp_obj3)
        delete tmp_obj3;
    fclose(fp);
}

//======================================================================
// get rgb-triple for named color
//======================================================================
int Application::get_color_rgb(float *r, float *g, float *b, char *color)
{
    char line[80];
    char *tp, *token[15];
    int count;
    const int tmax = 15;

    fseek(fp, 0L, SEEK_SET);
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        count = 0;
        tp = strtok(line, " \t");
        for (count = 0; count < tmax && tp != NULL;)
        {
            token[count] = tp;
            tp = strtok(NULL, " \t");
            count++;
        }
        token[count] = NULL;
        if (count == 5)
        {
            strcat(token[3], " ");
            strcat(token[3], token[4]);
        }
        if (strstr(token[3], color) != NULL)
        {
            *r = (float)atoi(token[0]) / 255.;
            *g = (float)atoi(token[1]) / 255.;
            *b = (float)atoi(token[2]) / 255.;
            fseek(fp, 0L, SEEK_SET);
            return (0);
        }
    }
    return (1);
}

void Application::genpolygons(char *GeometryN)
{
    int numlines, numpoints, numtriangles, numstrips;
    int linecoords, pointcoords, trianglecoords;
    int *pointlookup, *linelookup, *trianglelookup;
    int elem, setnum, i, iret;
    //float			*rp_out,*gp_out,*bp_out;
    //float			*rl_out,*gl_out,*bl_out;
    //float			*rs_out,*gs_out,*bs_out;
    float *p_x, *p_y, *p_z;
    float *l_x, *l_y, *l_z;
    float *s_x, *s_y, *s_z;
    float r, g, b;
    int *l_ll, *s_ll, *l_vl, *s_vl;
    char buf[300];
    numlines = 0;
    numpoints = 0;
    numtriangles = 0;
    numstrips = 0;
    linecoords = 0;
    pointcoords = 0;
    trianglecoords = 0;
    p_colors_s = NULL;
    p_colors_l = NULL;
    p_colors_p = NULL;
    pointlookup = new int[numcoord];
    linelookup = new int[numcoord];
    trianglelookup = new int[numcoord];
    for (i = 0; i < numcoord; i++)
        pointlookup[i] = linelookup[i] = trianglelookup[i] = -1;
    for (elem = 0; elem < numelem; elem++)
    {
        if (tl[elem] == TYPE_PRISM)
        {
            if (isflat != NULL)
            {
                if (strcmp(isflat, "true") == NULL)
                {
                    numtriangles += 2; // 2 triangles and 2 strips
                    numstrips += 2;
                }
                else if (strcmp(isflat, "TRUE") == NULL)
                {
                    numtriangles++; // 1 triangle and 1 strip
                    numstrips++;
                }
            }
            else
            {
                numtriangles += 8; // 6 triangles and 5 strips
                numstrips += 5;
            }
            for (i = 0; i < UnstructuredGrid_Num_Nodes[tl[elem]]; i++)
                if (trianglelookup[cl[el[elem] + i]] == -1)
                    trianglelookup[cl[el[elem] + i]] = trianglecoords++;
        }
        else
            switch (tl[elem])
            {
            case TYPE_POINT:
                numpoints++;
                if (pointlookup[cl[el[elem]]] == -1)
                    pointlookup[cl[el[elem]]] = pointcoords++;
                break;
            case TYPE_BAR:
                numlines++;
                if (linelookup[cl[el[elem]]] == -1)
                    linelookup[cl[el[elem]]] = linecoords++;
                if (linelookup[cl[el[elem] + 1]] == -1)
                    linelookup[cl[el[elem] + 1]] = linecoords++;
                break;
            case TYPE_HEXAGON:
                numtriangles += 4; // 12 triangles and 6 strips
                numstrips++;
            case TYPE_PRISM:
                numtriangles += 2; // 8 triangles and 5 strips
            case TYPE_PYRAMID:
                numtriangles += 2; // 6 triangles and 5 strips
                numstrips++;
            case TYPE_TETRAHEDER:
                numtriangles += 2; // 4 triangles and 4 stripe
                numstrips += 3;
            case TYPE_QUAD:
                numtriangles++; // 2 triangles and 1 stripe
            case TYPE_TRIANGLE:
                numtriangles++; // 1 triangle  and 1 stripe
                numstrips++;
                for (i = 0; i < UnstructuredGrid_Num_Nodes[tl[elem]]; i++)
                    if (trianglelookup[cl[el[elem] + i]] == -1)
                        trianglelookup[cl[el[elem] + i]] = trianglecoords++;
                break;
            }
    }
    if ((numtriangles == 0) && (numpoints == 0) && (numlines == 0))
    {
        Covise::sendError("ERROR: no valid elements found");
        return;
    }
    setnum = 0;
    Geometrys = new coDistributedObject *[4];
    Geometrys_num = 0;
    if (Geometrys != NULL)
    {
        if (ColorIn == NULL) // create one color object for all elements
        {
            iret = get_color_rgb(&r, &g, &b, cname);
            if (iret != 0)
            {
                r = 1.;
                g = 1.;
                b = 0.;
            }
            sprintf(buf, "%s_co", GeometryN);
            p_colors = new coDoRGBA(buf, 1);
            p_colors->setFloatRGBA(0, r, g, b, 1.0);
            p_colors->addAttribute("COLOR_BINDING", "OVERALL");
        }
        if (numpoints > 0)
        {
            sprintf(buf, "%s_%d", GeometryN, setnum++);
            Points = new coDoPoints(buf, numpoints);
            if (Points->objectOk())
            {
                Points->getAddresses(&p_x, &p_y, &p_z);
                Geometrys[Geometrys_num++] = Points;
            }
            else
            {
                Covise::sendError("ERROR: creation of Points object failed");
                return;
            }
            if (ColorIn != NULL)
            {
                sprintf(buf, "%s_cp", GeometryN);
                // u_colors_p = new coDoVec3(buf,numpoints);
                // u_colors_p->getAddresses(&rp_out, &gp_out, &bp_out);
                // u_colors_p->addAttribute("COLOR_BINDING", "PER_VERTEX");
                // color_sets_2[color_sets_2_num++]=u_colors_p;
                p_colors_p = new coDoRGBA(buf, numpoints);
                p_colors_p->addAttribute("COLOR_BINDING", "PER_VERTEX");
                color_sets_2[color_sets_2_num++] = p_colors_p;
            }
            else
            {
                color_sets_2[color_sets_2_num++] = p_colors;
            }
        }
        if (numlines > 0)
        {
            sprintf(buf, "%s_%d", GeometryN, setnum++);
            Lines = new coDoLines(buf, linecoords, numlines * 2, numlines);
            if (Lines->objectOk())
            {
                Lines->getAddresses(&l_x, &l_y, &l_z, &l_vl, &l_ll);
                for (i = 0; i < numlines; i++)
                    l_ll[i] = i * 2;
                Geometrys[Geometrys_num++] = Lines;
            }
            else
            {
                Covise::sendError("ERROR: creation of Lines object failed");
                return;
            }
            if (ColorIn != NULL)
            {
                sprintf(buf, "%s_cl", GeometryN);
                // u_colors_l = new coDoVec3(buf,linecoords);
                // u_colors_l->getAddresses(&rl_out, &gl_out, &bl_out);
                // u_colors_l->addAttribute("COLOR_BINDING", "PER_VERTEX");
                // color_sets_2[color_sets_2_num++]=u_colors_l;
                if (binding == 1)
                {
                    p_colors_l = new coDoRGBA(buf, linecoords);
                    p_colors_l->addAttribute("COLOR_BINDING", "PER_VERTEX");
                }
                if (binding == 2)
                {
                    p_colors_l = new coDoRGBA(buf, numlines);
                    p_colors_l->addAttribute("COLOR_BINDING", "PER_FACE");
                }
                color_sets_2[color_sets_2_num++] = p_colors_l;
            }
            else
            {
                color_sets_2[color_sets_2_num++] = p_colors;
            }
        }
        if (numtriangles > 0)
        {
            sprintf(buf, "%s_%d", GeometryN, setnum++);
            Strips = new coDoTriangleStrips(buf, trianglecoords, numtriangles + 2 * numstrips, numstrips);
            if (Strips->objectOk())
            {
                Strips->getAddresses(&s_x, &s_y, &s_z, &s_vl, &s_ll);
                Strips->addAttribute("vertexOrder", "2");
                Geometrys[Geometrys_num++] = Strips;
            }
            else
            {
                Covise::sendError("ERROR: creation of Strips object failed");
                return;
            }
            if (ColorIn != NULL)
            {
                sprintf(buf, "%s_cs", GeometryN);
                // u_colors_s = new coDoVec3(buf,trianglecoords);
                // u_colors_s->getAddresses(&rs_out, &gs_out, &bs_out);
                // u_colors_s->addAttribute("COLOR_BINDING", "PER_VERTEX");
                // color_sets_2[color_sets_2_num++]=u_colors_s;
                if (binding == 1)
                {
                    p_colors_s = new coDoRGBA(buf, trianglecoords);
                    p_colors_s->addAttribute("COLOR_BINDING", "PER_VERTEX");
                }
                if (binding == 2)
                {
                    p_colors_s = new coDoRGBA(buf, numtriangles);
                    p_colors_s->addAttribute("COLOR_BINDING", "PER_FACE");
                }
                color_sets_2[color_sets_2_num++] = p_colors_s;
            }
            else
            {
                color_sets_2[color_sets_2_num++] = p_colors;
            }
        }
        if (ColorIn == NULL) // increase refcount if we used the color obj multiple
        {
            if (setnum > 1)
                p_colors->incRefCount();
            if (setnum > 2)
                p_colors->incRefCount();
        }
    }
    else
    {
        Covise::sendError("ERROR: creation of data object 'geometry_set' failed");
        return;
    }
    numlines = 0;
    trianglecoords = 0;
    numstrips = 0;
    numtriangles = 0;
    for (elem = 0; elem < numelem; elem++)
    {

        switch (tl[elem])
        {
        case TYPE_POINT:
            p_x[pointlookup[cl[el[elem]]]] = x_in[cl[el[elem]]];
            p_y[pointlookup[cl[el[elem]]]] = y_in[cl[el[elem]]];
            p_z[pointlookup[cl[el[elem]]]] = z_in[cl[el[elem]]];
            if (ColorIn != NULL)
            {
                if (p_col != NULL)
                    p_colors_p->set_element_value(pointlookup[cl[el[elem]]], p_col[cl[el[elem]]]);
                else
                    p_colors_p->setFloatRGBA(pointlookup[cl[el[elem]]], r_in[cl[el[elem]]], g_in[cl[el[elem]]], b_in[cl[el[elem]]]);
            }
            break;
        case TYPE_BAR:
            l_vl[numlines * 2] = linelookup[cl[el[elem]]];
            l_vl[numlines * 2 + 1] = linelookup[cl[el[elem] + 1]];
            l_x[linelookup[cl[el[elem]]]] = x_in[cl[el[elem]]];
            l_y[linelookup[cl[el[elem]]]] = y_in[cl[el[elem]]];
            l_z[linelookup[cl[el[elem]]]] = z_in[cl[el[elem]]];
            l_x[linelookup[cl[el[elem] + 1]]] = x_in[cl[el[elem] + 1]];
            l_y[linelookup[cl[el[elem] + 1]]] = y_in[cl[el[elem] + 1]];
            l_z[linelookup[cl[el[elem] + 1]]] = z_in[cl[el[elem] + 1]];
            if (ColorIn != NULL)
            {
                if (binding == 1)
                {
                    if (p_col != NULL)
                    {
                        p_colors_l->set_element_value(linelookup[cl[el[elem]]], p_col[cl[el[elem]]]);
                        p_colors_l->set_element_value(linelookup[cl[el[elem] + 1]], p_col[cl[el[elem]]]);
                    }
                    else
                    {
                        p_colors_l->setFloatRGBA(linelookup[cl[el[elem]]], r_in[cl[el[elem]]], g_in[cl[el[elem]]], b_in[cl[el[elem]]]);
                        p_colors_l->setFloatRGBA(linelookup[cl[el[elem] + 1]], r_in[cl[el[elem] + 1]], g_in[cl[el[elem] + 1]], b_in[cl[el[elem] + 1]]);
                    }
                }
                if (binding == 2)
                {
                    if (p_col != NULL)
                        p_colors_l->set_element_value(numlines, p_col[elem]);
                    else
                        p_colors_l->setFloatRGBA(numlines, r_in[elem], g_in[elem], b_in[elem]);
                }
            }
            numlines++;
            break;
        case TYPE_HEXAGON: // 12 triangles and 6 strips
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 5]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 5]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 6]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 6]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 7]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 7]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 0]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 5]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 6]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 7]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            numstrips++;
            for (i = 0; i < 8; i++)
            {
                s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                if ((ColorIn != NULL) && (binding == 1))
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                    else
                        p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                }
            }
            if ((binding == 2) && (ColorIn != NULL))
            {
                for (i = 0; i < 12; i++)
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(numtriangles + i, p_col[elem]);
                    else
                        p_colors_s->setFloatRGBA(numtriangles + i, r_in[elem], g_in[elem], b_in[elem]);
                }
            }
            numtriangles += 12;
            break;
        case TYPE_PRISM: // 8 triangles and 5 strips
            if ((isflat != NULL) && (strcmp(isflat, "true") == NULL))
            {
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
                trianglecoords++;
                numstrips++;
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 5]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
                trianglecoords++;
                numstrips++;
                for (i = 0; i < 6; i++)
                {
                    s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                    s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                    s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                    if ((ColorIn != NULL) && (binding == 1))
                    {
                        if (p_col != NULL)
                            p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                        else
                            p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                    }
                }
                if ((binding == 2) && (ColorIn != NULL))
                {
                    for (i = 0; i < 2; i++)
                    {
                        if (p_col != NULL)
                            p_colors_s->set_element_value(numtriangles + i, p_col[elem]);
                        else
                            p_colors_s->setFloatRGBA(numtriangles + i, r_in[elem], g_in[elem], b_in[elem]);
                    }
                }
                numtriangles += 2;
            }
            else if ((isflat != NULL) && (strcmp(isflat, "TRUE") == NULL))
            {
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
                trianglecoords++;
                numstrips++;
                for (i = 0; i < 3; i++)
                {
                    s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                    s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                    s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                    if ((ColorIn != NULL) && (binding == 1))
                    {
                        if (p_col != NULL)
                            p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                        else
                            p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                    }
                }
                if ((binding == 2) && (ColorIn != NULL))
                {

                    if (p_col != NULL)
                        p_colors_s->set_element_value(numtriangles, p_col[elem]);
                    else
                        p_colors_s->setFloatRGBA(numtriangles, r_in[elem], g_in[elem], b_in[elem]);
                }
                numtriangles++;
            }
            else
            {
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
                trianglecoords++;
                numstrips++;
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 5]];
                trianglecoords++;
                numstrips++;
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 5]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
                trianglecoords++;
                numstrips++;
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
                trianglecoords++;
                numstrips++;
                s_ll[numstrips] = trianglecoords;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 5]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
                trianglecoords++;
                s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
                trianglecoords++;
                numstrips++;
                for (i = 0; i < 6; i++)
                {
                    s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                    s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                    s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                    if ((ColorIn != NULL) && (binding == 1))
                    {
                        if (p_col != NULL)
                            p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                        else
                            p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                    }
                }
                if ((binding == 2) && (ColorIn != NULL))
                {
                    for (i = 0; i < 8; i++)
                    {
                        if (p_col != NULL)
                            p_colors_s->set_element_value(numtriangles + i, p_col[elem]);
                        else
                            p_colors_s->setFloatRGBA(numtriangles + i, r_in[elem], g_in[elem], b_in[elem]);
                    }
                }
                numtriangles += 8;
            }
            break;
        case TYPE_PYRAMID: // 6 triangles and 5 strips
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 4]];
            trianglecoords++;
            numstrips++;
            for (i = 0; i < 5; i++)
            {
                s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                if ((ColorIn != NULL) && (binding == 1))
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                    else
                        p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                }
            }
            if ((binding == 2) && (ColorIn != NULL))
            {
                for (i = 0; i < 6; i++)
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(numtriangles + i, p_col[elem]);
                    else
                        p_colors_s->setFloatRGBA(numtriangles + i, r_in[elem], g_in[elem], b_in[elem]);
                }
            }
            numtriangles += 6;
            break;
        case TYPE_TETRAHEDER: // 4 triangles and 4 stripe
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            numstrips++;
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            numstrips++;
            for (i = 0; i < 4; i++)
            {
                s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                if ((ColorIn != NULL) && (binding == 1))
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                    else
                        p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                }
            }
            if ((binding == 2) && (ColorIn != NULL))
            {
                for (i = 0; i < 4; i++)
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(numtriangles + i, p_col[elem]);
                    else
                        p_colors_s->setFloatRGBA(numtriangles + i, r_in[elem], g_in[elem], b_in[elem]);
                }
            }
            numtriangles += 4;
            break;
        case TYPE_QUAD: // 2 triangles and 1 stripe
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 3]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            numstrips++;
            for (i = 0; i < 4; i++)
            {
                s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                if ((ColorIn != NULL) && (binding == 1))
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                    else
                        p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                }
            }
            if ((binding == 2) && (ColorIn != NULL))
            {
                if (p_col != NULL)
                {
                    p_colors_s->set_element_value(numtriangles, p_col[elem]);
                    p_colors_s->set_element_value(numtriangles + 1, p_col[elem]);
                }
                else
                {
                    p_colors_s->setFloatRGBA(numtriangles, r_in[elem], g_in[elem], b_in[elem]);
                    p_colors_s->setFloatRGBA(numtriangles + 1, r_in[elem], g_in[elem], b_in[elem]);
                }
            }
            numtriangles += 2;
            break;
        case TYPE_TRIANGLE: // 1 triangle  and 1 stripe
            s_ll[numstrips] = trianglecoords;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem]]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 1]];
            trianglecoords++;
            s_vl[trianglecoords] = trianglelookup[cl[el[elem] + 2]];
            trianglecoords++;
            numstrips++;
            for (i = 0; i < 3; i++)
            {
                s_x[trianglelookup[cl[el[elem] + i]]] = x_in[cl[el[elem] + i]];
                s_y[trianglelookup[cl[el[elem] + i]]] = y_in[cl[el[elem] + i]];
                s_z[trianglelookup[cl[el[elem] + i]]] = z_in[cl[el[elem] + i]];
                if ((ColorIn != NULL) && (binding == 1))
                {
                    if (p_col != NULL)
                        p_colors_s->set_element_value(trianglelookup[cl[el[elem] + i]], p_col[cl[el[elem] + i]]);
                    else
                        p_colors_s->setFloatRGBA(trianglelookup[cl[el[elem] + i]], r_in[cl[el[elem] + i]], g_in[cl[el[elem] + i]], b_in[cl[el[elem] + i]]);
                }
            }
            if ((binding == 2) && (ColorIn != NULL))
            {
                if (p_col != NULL)
                    p_colors_s->set_element_value(numtriangles, p_col[elem]);
                else
                    p_colors_s->setFloatRGBA(numtriangles, r_in[elem], g_in[elem], b_in[elem]);
            }
            numtriangles++;
            break;
        }
    }
    delete pointlookup;
    delete linelookup;
    delete trianglelookup;
    /*if(u_colors_s!=NULL)
       delete u_colors_s;
   if(u_colors_l!=NULL)
       delete u_colors_l;
   if(u_colors_p!=NULL)
       delete u_colors_p;*/
    Geometrys[Geometrys_num] = NULL;
    Geometry_set = new coDoSet(GeometryN, (coDistributedObject **)Geometrys);
    for (i = 0; i < Geometrys_num; i++)
        delete Geometrys[i];
    delete[] Geometrys;
}
