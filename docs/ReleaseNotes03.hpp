// -*- latex -*-

/**
\page page_release_notes_03 Release Notes RobWork Version 0.3

\section sec_release_notes_03_major Major Changes


- All static methods start with lower-case. Most of the old methods are still available but are
deprecated and will be removed in later releases.

- New package rw::trajectory is replacing the old rw::interpolator package.

- Drawables are splitted into a Render part and a thin Drawable class. Renders are cached and
reused to save memory and load time.

- Dynamic inserting and removing of frames, enabling users to modify the workcell at runtime.
This feature is still to be considered at a beta state.

- All device and sensor drivers are moved into a seperate project called RobWorkHardware,
which will be release soon. The version number of RobWorkHardware will follow the version number
of RobWork.

- A Task format has been introduced. This format may still be subject to change.

- A factory for loading workcells has been introduced (rw::loaders::WorkCellLoader)

- The collision detection strategy based on Opcode has been removed, because of compile issues
and problems with invalid results. Instead a collision detector called Yaobi is introduced.

- New structure for motion planners and abstraction of constraints, samplers etc. for planning

- Introduction of SBL and ARW motion planners

- Introduction of the RobWork smart-pointer rw::common::Ptr, which handles optional ownership.
This pointer is used in most of the new interfaces and several of the old once to make it
optional whether to transfer ownership of an object.

- Introduction of rw::math::MetricFactory for constructing metrics.

- Interfaces for simulated devices and sensors. This is still in a beta state.

- A directory acting as a sandbox for experimental code and code under development. Whether to
compile the sandbox is specified in the RobWork.cmake file.

- New CMakeFile structure to make it easier to configure RobWork, RobWorkStudio and other
projects with the same setup.

*/
