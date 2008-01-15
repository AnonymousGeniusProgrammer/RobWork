/*********************************************************************
 * RobWork Version 0.2
 * Copyright (C) Robotics Group, Maersk Institute, University of Southern
 * Denmark.
 *
 * RobWork can be used, modified and redistributed freely.
 * RobWork is distributed WITHOUT ANY WARRANTY; including the implied
 * warranty of merchantability, fitness for a particular purpose and
 * guarantee of future releases, maintenance and bug fixes. The authors
 * has no responsibility of continuous development, maintenance, support
 * and insurance of backwards capability in the future.
 *
 * Notice that RobWork uses 3rd party software for which the RobWork
 * license does not apply. Consult the packages in the ext/ directory
 * for detailed information about these packages.
 *********************************************************************/
#include "Drawable3DS.hpp"

#include <rw/common/macros.hpp>

using namespace rwlibs::drawable;

Drawable3DS::Drawable3DS(const std::string &filename)
{
    RW_ASSERT(!filename.empty());

    _model.Load(filename); // Load the model
    _displayListId = glGenLists(1);
    glNewList(_displayListId, GL_COMPILE);
    glPushMatrix();
    _model.Draw();
    glPopMatrix();
    glEndList();
}

void Drawable3DS::update(UpdateType update) {}
