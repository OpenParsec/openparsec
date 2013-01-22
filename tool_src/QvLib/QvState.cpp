
//[MSH_START]
// 02/01/98:	moved FetchCoordinate3State(), FetchMaterialState(), and
//				FetchMaterialBindingState() into class BRep.
// 01/01/98:	added new material retrieval code (msh)
// 05/05/97:	added new constructor (msh)
// 04/03/97:	added display of coordinate3 stack (msh)
//[MSH_END]

#include <QvState.h>


const char *QvState::stackNames[NumStacks] = {
    "Camera",
    "Coordinate3",
    "Light",
    "MaterialBinding",
    "Material",
    "NormalBinding",
    "Normal",
    "ShapeHints",
    "Texture2",
    "Texture2Transformation",
    "TextureCoordinate2",
    "Transformation",
};

//[MSH_START]
QvState::QvState( BSPLIB_NAMESPACE::VrmlFile *base )
{
	vrmlfile_base = base;
	//stacks = new QvElement[NumStacks];

    //stacks = (QvElement **) new QvElement[NumStacks];
	//stacks = new QvElement * [NumStacks];

	for (int i = 0; i < NumStacks; i++){
		stacks[i] = NULL;
	}
    depth = 0;
}
//[MSH_END]

QvState::QvState()
{
//[MSH_START]
	vrmlfile_base = NULL;
//[MSH_END]

    //stacks = new QvElement[NumStacks];
	//stacks = (QvElement **) new QvElement[NumStacks];
	//stacks = new QvElement * [NumStacks];

	int i = 0;
    for (i = 0; i < NumStacks; i++){
		stacks[i] = NULL;
	}
    depth = 0;
}

QvState::~QvState()
{
    while (depth > 0)
	pop();

    //delete [] stacks;
}

void
QvState::addElement(StackIndex stackIndex, QvElement *elt)
{
    elt->depth = depth;
    elt->next = stacks[stackIndex];
    stacks[stackIndex] = elt;
}

void
QvState::push()
{
    depth++;
}

void
QvState::pop()
{
    depth--;

    for (int i = 0; i < NumStacks; i++)
	while (stacks[i] != NULL && stacks[i]->depth > depth)
	    popElement((StackIndex) i);
}

void
QvState::popElement(StackIndex stackIndex)
{
    QvElement *elt = stacks[stackIndex];
    stacks[stackIndex] = elt->next;
    delete elt;
}

void
QvState::print()
{
    printf("Traversal state:\n");

    for (int i = 0; i < NumStacks; i++) {
		printf("\tStack [%2d] (%s):\n", i, stackNames[i]);

		if (stacks[i] == NULL){
			printf("\t\tNULL\n");
		} else {
			for (QvElement *elt = stacks[i]; elt != NULL; elt = elt->next) {
				elt->print();
//[MSH_START]
/*
				if ( i == Coordinate3Index ) {
					QvCoordinate3 *c3 = (QvCoordinate3 *) elt->data;
					for ( int j = 0; j < c3->point.num; j++ )
						printf( "point[%d]=%f %f %f\n", j, c3->point.values[ j*3 ], c3->point.values[ j*3 + 1 ], c3->point.values[ j*3 + 2 ] );

				}
*/
//[MSH_END]
			}
		}
	
    }
}
