/***************************************
 A header file for the nodes.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#ifndef NODES_H
#define NODES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "files.h"
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single node. +*/
struct _Node
{
 index_t      firstseg;         /*+ The index of the first segment. +*/

 ll_off_t     latoffset;        /*+ The node latitude offset within its bin. +*/
 ll_off_t     lonoffset;        /*+ The node longitude offset within its bin. +*/

 transports_t allow;            /*+ The types of transport that are allowed through the node. +*/
 uint16_t     flags;            /*+ Flags containing extra information (e.g. super-node, turn restriction). +*/
};


/*+ A structure containing the header from the file. +*/
typedef struct _NodesFile
{
 index_t  number;               /*+ The number of nodes in total. +*/
 index_t  snumber;              /*+ The number of super-nodes. +*/

 ll_bin_t latbins;              /*+ The number of bins containing latitude. +*/
 ll_bin_t lonbins;              /*+ The number of bins containing longitude. +*/

 ll_bin_t latzero;              /*+ The bin number of the furthest south bin. +*/
 ll_bin_t lonzero;              /*+ The bin number of the furthest west bin. +*/
}
 NodesFile;


/*+ A structure containing a set of nodes. +*/
struct _Nodes
{
 NodesFile file;                /*+ The header data from the file. +*/

#if !SLIM

 void     *data;                /*+ The memory mapped data in the file. +*/

 index_t  *offsets;             /*+ A pointer to the array of offsets in the file. +*/

 Node     *nodes;               /*+ A pointer to the array of nodes in the file. +*/

#else

 int       fd;                  /*+ The file descriptor for the file. +*/
 off_t     nodesoffset;         /*+ The offset of the nodes within the file. +*/

 Node      cached[4];           /*+ Four cached nodes read from the file in slim mode. +*/
 index_t   incache[4];          /*+ The indexes of the cached nodes. +*/

#endif
};


/* Functions in nodes.c */

Nodes *LoadNodeList(const char *filename);

index_t FindClosestNode(Nodes *nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                        distance_t distance,Profile *profile,distance_t *bestdist);

index_t FindClosestSegment(Nodes *nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                           distance_t distance,Profile *profile, distance_t *bestdist,
                           index_t *bestnode1,index_t *bestnode2,distance_t *bestdist1,distance_t *bestdist2);

void GetLatLong(Nodes *nodes,index_t index,double *latitude,double *longitude);


/* Macros and inline functions */

/*+ Return true if this is a super-node. +*/
#define IsSuperNode(xxx)            (((xxx)->flags)&NODE_SUPER)

/*+ Return true if this is a turn restricted node. +*/
#define IsTurnRestrictedNode(xxx)   (((xxx)->flags)&NODE_TURNRSTRCT)


#if !SLIM

/*+ Return a Node pointer given a set of nodes and an index. +*/
#define LookupNode(xxx,yyy,ppp)       (&(xxx)->nodes[yyy])

/*+ Return a Segment index given a Node pointer and a set of segments. +*/
#define FirstSegment(xxx,yyy,zzz,ppp) LookupSegment((xxx),(yyy)->nodes[zzz].firstseg,1)

/*+ Return the offset of a geographical region given a set of nodes. +*/
#define LookupNodeOffset(xxx,yyy)     ((xxx)->offsets[yyy])

#else

static Node *LookupNode(Nodes *nodes,index_t index,int position);

/*+ Return a Segment index given a Node pointer and a set of segments. +*/
#define FirstSegment(xxx,yyy,zzz,ppp) LookupSegment((xxx),FirstSegment_internal(yyy,zzz),ppp)

static index_t FirstSegment_internal(Nodes *nodes,index_t index);

static index_t LookupNodeOffset(Nodes *nodes,index_t index);


/*++++++++++++++++++++++++++++++++++++++
  Find the Node information for a particular node.

  Node *LookupNode Returns a pointer to the cached node information.

  Nodes *nodes The set of nodes to use.

  index_t index The index of the node.

  int position The position in the cache to store the value.
  ++++++++++++++++++++++++++++++++++++++*/

static inline Node *LookupNode(Nodes *nodes,index_t index,int position)
{
 if(nodes->incache[position-1]!=index)
   {
    SeekFile(nodes->fd,nodes->nodesoffset+(off_t)index*sizeof(Node));

    ReadFile(nodes->fd,&nodes->cached[position-1],sizeof(Node));

    nodes->incache[position-1]=index;
   }

 return(&nodes->cached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the index of the first segment of a node (called by FirstSegment() macro).

  index_t FirstSegment_internal Returns the index of the first segment.

  Nodes *nodes The set of nodes to use.

  index_t index The index of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static inline index_t FirstSegment_internal(Nodes *nodes,index_t index)
{
 Node *node;

 node=LookupNode(nodes,index,4);

 return(node->firstseg);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the offset of nodes in a geographical region.

  index_t LookupNodeOffset Returns the index offset.

  Nodes *nodes The set of nodes to use.

  index_t index The index of the offset.
  ++++++++++++++++++++++++++++++++++++++*/

static inline index_t LookupNodeOffset(Nodes *nodes,index_t index)
{
 index_t offset;

 SeekFile(nodes->fd,sizeof(NodesFile)+(off_t)index*sizeof(index_t));

 ReadFile(nodes->fd,&offset,sizeof(index_t));

 return(offset);
}

#endif


#endif /* NODES_H */
