#include "abstract.h"


// ---------------------------------------------------------------------------------------------------------------------
// Executables

/// type of executable, either a single line (Operation), or list of lines (Block)
enum class ExecutableType
{
    Operation,
    Block
};

/// abstract class for anything build from operations
class Executable 
{
    public:
    ExecutableType ExecType;
};



/// a sequential list of atomic operations/Blocks is a block
/// [Executables] is the list child Execuable objects
class Block : public Executable
{
    public:
    std::vector<Executable*> Executables;
};

