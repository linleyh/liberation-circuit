
#process "outpost"

class auto_harvest;
class auto_allocate;

core_static_quad, 811, 
  {object_harvest:auto_harvest, 0},
  {object_none, 0},
  {object_allocate:auto_allocate, 0},
  {object_storage, 0},

#code

int initialised;

if (!initialised)
{
 initialised = 1;
 special_AI(0, 104);
}

auto_harvest.gather_data();

auto_allocate.allocate_data(4); // actually I think the maximum is 2
