#include "Test.hpp"

#include <Concepts.hpp>

using namespace CSTM;

DeclTest(concepts, has_return_type)
{
	Cond(Eq, (HasReturnType<int(*)(), int>), true);
	Cond(Eq, (HasReturnType<float(*)(), int>), false);
}
