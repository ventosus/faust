/************************************************************************
 ************************************************************************
    FAUST compiler
    Copyright (C) 2003-2015 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ************************************************************************
 ************************************************************************/

#ifndef _INTERPRETER_INSTRUCTIONS_H
#define _INTERPRETER_INSTRUCTIONS_H

using namespace std;

#include "instructions.hh"
#include "typing_instructions.hh"
#include "fir_interpreter.hh"

template <class T>
struct InterpreterInstVisitor : public DispatchVisitor {

        /*
         Global functions names table as a static variable in the visitor
         so that each function prototype is generated as most once in the module.
         */
        static map <string, FIRInstruction::Opcode> gMathLibTable;
        
        int fRealHeapOffset;    // Offset in Real HEAP    
        int fIntHeapOffset;     // Offset in Integer HEAP
        int fSROffset;          // Offset in Integer HEAP for "fSamplingFreq"
    
        map <string, pair<int, Typed::VarType> > fFieldTable;   // Table : field_name, <byte offset in structure, type>
          
        TypingVisitor fTypingVisitor;
   
        FIRUserInterfaceBlockInstruction<T>* fUserInterfaceBlock;
        FIRBlockInstruction<T>* fCurrentBlock;
   
        InterpreterInstVisitor()
        {
            fUserInterfaceBlock = new FIRUserInterfaceBlockInstruction<T>();
            fCurrentBlock = new FIRBlockInstruction<T>();
            fRealHeapOffset = 0;
            fIntHeapOffset = 0;
            fSROffset = 0;
            initMathTable();
        }
    
        inline bool isIntType(Typed::VarType type)
        {
            return (type == Typed::kInt);
        }
    
        inline bool isRealType(Typed::VarType type)
        { 
            return (type == Typed::kFloat 
                || type == Typed::kFloatMacro 
                || type == Typed::kDouble); 
        }
        
        inline bool isRealPtrType(Typed::VarType type) 
        { 
            return (type == Typed::kFloat_ptr 
                || type == Typed::kFloatMacro_ptr 
                || type == Typed::kDouble_ptr); 
        }
        
        inline bool isInternalRealType(Typed::VarType type) 
        { 
            return (type == Typed::kFloat || type == Typed::kDouble); 
        }
    
        void initMathTable()
        {
            // TODO
            //fMathLibTable["abs"] = "global.Math.abs";
            //fMathLibTable["absf"] = "global.Math.abs";
            //fMathLibTable["fabsf"] = "global.Math.abs";
            //fMathLibTable["acosf"] = "global.Math.acos";
            //fMathLibTable["asinf"] = "global.Math.asin";
            //fMathLibTable["atanf"] = "global.Math.atan";
            //fMathLibTable["atan2f"] = "global.Math.atan2";
            //fMathLibTable["ceilf"] = "global.Math.ceil";
            gMathLibTable["cosf"] = FIRInstruction::kCos;
            //fMathLibTable["expf"] = "global.Math.exp";
            //fMathLibTable["floorf"] = "global.Math.floor";
            //fMathLibTable["fmodf"] = "manual";      // Manually generated
            //fMathLibTable["logf"] = "global.Math.log";
            //fMathLibTable["log10f"] = "manual";     // Manually generated
            //fMathLibTable["max"] = "global.Math.max";
            //fMathLibTable["min"] = "global.Math.min";
            //fMathLibTable["powf"] = "global.Math.pow";
            //fMathLibTable["roundf"] = "global.Math.round";
            gMathLibTable["sinf"] = FIRInstruction::kSin;
            gMathLibTable["sqrtf"] = FIRInstruction::kSqrt;
            //fMathLibTable["tanf"] = "global.Math.tan";
        }
        
        virtual ~InterpreterInstVisitor()
        {}
   
        virtual void visit(AddMetaDeclareInst* inst)
        {
            pair<int, Typed::VarType> tmp = fFieldTable[inst->fZone];
            if (inst->fZone == "0") {
                fUserInterfaceBlock->push(new FIRUserInterfaceInstruction<T>(FIRInstruction::kDeclare, -1, inst->fKey, inst->fValue));
            } else {
                fUserInterfaceBlock->push(new FIRUserInterfaceInstruction<T>(FIRInstruction::kDeclare, tmp.first, inst->fKey, inst->fValue));
            }
        }

        virtual void visit(OpenboxInst* inst)
        {
            FIRInstruction::Opcode opcode;
            switch (inst->fOrient) {
                case 0:
                    opcode = FIRInstruction::kOpenVerticalBox;
                    break;
                case 1:
                    opcode = FIRInstruction::kOpenHorizontalBox;
                    break;
                case 2:
                    opcode = FIRInstruction::kOpenTabBox;
                    break;
            }
            
            fUserInterfaceBlock->push(new FIRUserInterfaceInstruction<T>(opcode, inst->fName));
        }

        virtual void visit(CloseboxInst* inst)
        {
            fUserInterfaceBlock->push(new FIRUserInterfaceInstruction<T>(FIRInstruction::kCloseBox));
        }
        
        virtual void visit(AddButtonInst* inst)
        {
            FIRInstruction::Opcode opcode;
            if (inst->fType == AddButtonInst::kDefaultButton) {
                opcode = FIRInstruction::kAddButton;
            } else {
                opcode = FIRInstruction::kAddCheckButton;
            }
            
            pair<int, Typed::VarType> tmp = fFieldTable[inst->fZone];
            fUserInterfaceBlock->push(new FIRUserInterfaceInstruction<T>(opcode, tmp.first, inst->fLabel));
        }

        virtual void visit(AddSliderInst* inst)
        {
            FIRInstruction::Opcode opcode;
            switch (inst->fType) {
                case AddSliderInst::kHorizontal:
                    opcode = FIRInstruction::kAddHorizontalSlider;
                    break;
                case AddSliderInst::kVertical:
                    opcode = FIRInstruction::kAddVerticalSlider;
                    break;
                case AddSliderInst::kNumEntry:
                    opcode = FIRInstruction::kAddNumEntry;
                    break;
            }
        
            pair<int, Typed::VarType> tmp = fFieldTable[inst->fZone];
            fUserInterfaceBlock->push(new FIRUserInterfaceInstruction<T>(opcode, tmp.first, inst->fLabel, inst->fInit, inst->fMin, inst->fMax, inst->fStep));
        }

        virtual void visit(AddBargraphInst* inst)
        {
            FIRInstruction::Opcode opcode;
            switch (inst->fType) {
                case AddBargraphInst::kHorizontal:
                    opcode = FIRInstruction::kAddHorizontalBargraph;
                    break;
                case AddBargraphInst::kVertical:
                    opcode = FIRInstruction::kAddVerticalBargraph;
                    break;
            }
            
            pair<int, Typed::VarType> tmp = fFieldTable[inst->fZone];
            fUserInterfaceBlock->push(new FIRUserInterfaceInstruction<T>(opcode, tmp.first, inst->fLabel, inst->fMin, inst->fMax));
        }
        
        virtual void visit(LabelInst* inst) {}

        // Declarations
        virtual void visit(DeclareVarInst* inst) 
        {
            // HACK : completely adhoc code for input/output...
            if ((startWith(inst->fAddress->getName(), "input") || startWith(inst->fAddress->getName(), "output"))) {
                return;
            }
            
            ArrayTyped* array_typed = dynamic_cast<ArrayTyped*>(inst->fType);
            
            if (array_typed && array_typed->fSize > 1) {
                if (array_typed->fType->getType() == Typed::kInt) {
                    fFieldTable[inst->fAddress->getName()] = make_pair(fIntHeapOffset, array_typed->fType->getType());
                    fIntHeapOffset += array_typed->fSize;
                } else {
                    fFieldTable[inst->fAddress->getName()] = make_pair(fRealHeapOffset, array_typed->fType->getType());
                    fRealHeapOffset += array_typed->fSize;
                }
            } else {
                if (inst->fType->getType() == Typed::kInt) {
                    // Keep "fSamplingFreq" offset
                    if (inst->fAddress->getName() == "fSamplingFreq") fSROffset = fIntHeapOffset;
                    fFieldTable[inst->fAddress->getName()] = make_pair(fIntHeapOffset, inst->fType->getType());
                    fIntHeapOffset++;
                } else {
                    fFieldTable[inst->fAddress->getName()] = make_pair(fRealHeapOffset, inst->fType->getType());
                    fRealHeapOffset++;
                }
            }
            
            // Simulate a 'Store'
            if (inst->fValue) {
                visitStore(inst->fAddress, inst->fValue);
            }
        }
        
        virtual void visit(DeclareFunInst* inst) {}
    
        // Memory
    
    
        virtual void visit(LoadVarInst* inst) 
        {
            fTypingVisitor.visit(inst);
            
            // Compile address
            inst->fAddress->accept(this);
            
            NamedAddress* named = dynamic_cast<NamedAddress*>(inst->fAddress);
            IndexedAddress* indexed = dynamic_cast<IndexedAddress*>(inst->fAddress);
            
            pair<int, Typed::VarType> tmp = fFieldTable[inst->fAddress->getName()];
            
            if (named) {
                fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt) 
                                    ? FIRInstruction::kLoadInt : FIRInstruction::kLoadReal, 0, 0, tmp.first, 0));
            } else {
                // Indexed 
                string num;
                if (startWithRes(indexed->getName(), "input", num)) {
                    fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kLoadInput, 0, 0, atoi(num.c_str()), 0));
                } else {
                    fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt) 
                                        ? FIRInstruction::kLoadIndexedInt : FIRInstruction::kLoadIndexedReal, 0, 0, tmp.first, 0));
                
                }
            }
        }
    
        /*
        virtual void visit(LoadVarInst* inst)
        {
            fTypingVisitor.visit(inst);
            
            NamedAddress* named = dynamic_cast<NamedAddress*>(inst->fAddress);
            IndexedAddress* indexed = dynamic_cast<IndexedAddress*>(inst->fAddress);
            
            pair<int, Typed::VarType> tmp = fFieldTable[inst->fAddress->getName()];
            
            if (named) {
                fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt)
                                                               ? FIRInstruction::kLoadInt : FIRInstruction::kLoadReal, 0, 0, tmp.first));
            } else {
                // Indexed
                string num;
                if (startWithRes(indexed->getName(), "input", num)) {
                    // Compile address
                    inst->fAddress->accept(this);
                    fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kLoadInput, 0, 0, atoi(num.c_str())));
                } else {
                    // If index is a known value, we can directly compute the final offset and generate a normal Load
                    IntNumInst* num_inst = dynamic_cast<IntNumInst*>(indexed->fIndex);
                    if (num_inst) {
                        fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt)
                                                                       ? FIRInstruction::kLoadInt : FIRInstruction::kLoadReal,
                                                                       0, 0,
                                                                       tmp.first + num_inst->fNum
                                                                       ));
                        
                    } else {
                        // Compile address
                        inst->fAddress->accept(this);
                        fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt)
                                                                   ? FIRInstruction::kLoadIndexedInt : FIRInstruction::kLoadIndexedReal, 0, 0, tmp.first));
                    }
                    
                }
            }
        }
         */
    
        //virtual void visit(LoadVarAddressInst* inst) {}
    
    
        virtual void visitStore(Address* address, ValueInst* value)
        {
            // Compile value
            value->accept(this);
            
            NamedAddress* named = dynamic_cast<NamedAddress*>(address);
            IndexedAddress* indexed = dynamic_cast<IndexedAddress*>(address);
            
            pair<int, Typed::VarType> tmp = fFieldTable[address->getName()];
            
            if (named) {
                fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt) 
                                    ? FIRInstruction::kStoreInt : FIRInstruction::kStoreReal, 0, 0, tmp.first, 0));
            } else {
                // Compile  address
                address->accept(this);
                // Indexed 
                string num;
                if (startWithRes(indexed->getName(), "output", num)) {
                    fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kStoreOutput, 0, 0, atoi(num.c_str()), 0));
                } else {
                    fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt) 
                                        ? FIRInstruction::kStoreIndexedInt : FIRInstruction::kStoreIndexedReal, 0, 0, tmp.first, 0));
                }
            }
        }
    
        /*
        virtual void visitStore(Address* address, ValueInst* value)
        {
            
            NamedAddress* named = dynamic_cast<NamedAddress*>(address);
            IndexedAddress* indexed = dynamic_cast<IndexedAddress*>(address);
            
            pair<int, Typed::VarType> tmp = fFieldTable[address->getName()];
            
            if (named) {
                // Compile value
                value->accept(this);
                fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt)
                                                               ? FIRInstruction::kStoreInt : FIRInstruction::kStoreReal, 0, 0, tmp.first));
            } else {
                // Indexed
                string num;
                if (startWithRes(indexed->getName(), "output", num)) {
                    // Compile address and value
                    address->accept(this);
                    value->accept(this);
                    fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kStoreOutput, 0, 0, atoi(num.c_str())));
                } else {
                    // If index is a known value, we can directly compute the final offset and generate a normal Store
                    IntNumInst* num_inst = dynamic_cast<IntNumInst*>(indexed->fIndex);
                    if (num_inst) {
                        // Compile value
                        value->accept(this);
                        fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt)
                                                                       ? FIRInstruction::kStoreInt : FIRInstruction::kStoreReal,
                                                                       0, 0,
                                                                       tmp.first + num_inst->fNum
                                                                       ));
                    } else {
                        // Compile address and value
                        address->accept(this);
                        value->accept(this);
                        fCurrentBlock->push(new FIRBasicInstruction<T>((tmp.second == Typed::kInt)
                                                                   ? FIRInstruction::kStoreIndexedInt : FIRInstruction::kStoreIndexedReal, 0, 0, tmp.first));
                    }
                }
            }
        }
         */

        virtual void visit(StoreVarInst* inst)
        {
            visitStore(inst->fAddress, inst->fValue);
        }

        // Addresses
        virtual void visit(NamedAddress* named)
        {
            // Nothing
        }
        
        virtual void visit(IndexedAddress* indexed) 
        {   
            indexed->fIndex->accept(this);
        }

        // Primitives : numbers
        virtual void visit(FloatNumInst* inst) 
        {
            fTypingVisitor.visit(inst);
            
            fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kRealValue, 0, inst->fNum));
        }
        
        virtual void visit(FloatArrayNumInst* inst) {}
        
        virtual void visit(IntNumInst* inst)  
        {
            fTypingVisitor.visit(inst);
            
            fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kIntValue, inst->fNum, 0));
        }
        
        virtual void visit(IntArrayNumInst* inst) {}
        
        virtual void visit(BoolNumInst* inst)
        {
            fTypingVisitor.visit(inst);
            
            fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kIntValue, inst->fNum, 0));
        }
        
        virtual void visit(DoubleNumInst* inst) 
        {
            fTypingVisitor.visit(inst);
            
            // Double considered real...
            fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kRealValue, 0, inst->fNum));
        }
        
        virtual void visit(DoubleArrayNumInst* inst) {}

        // Numerical computation
        virtual void visit(BinopInst* inst) 
        {
            // Compile arguments in reverse order
            inst->fInst2->accept(this);
            inst->fInst1->accept(this);
            
            inst->fInst1->accept(&fTypingVisitor);
            Typed::VarType type1 = fTypingVisitor.fCurType;
            
            inst->fInst2->accept(&fTypingVisitor);
            Typed::VarType type2 = fTypingVisitor.fCurType;
            
            if (isRealType(type1) || isRealType(type2)) {
                fCurrentBlock->push(new FIRBasicInstruction<T>(gBinOpTable[inst->fOpcode]->fInterpFloatInst));
            } else if (isIntType(type1) || isIntType(type2)) {
                fCurrentBlock->push(new FIRBasicInstruction<T>(gBinOpTable[inst->fOpcode]->fInterpIntInst));
            } else if (type1 == Typed::kBool && type2 == Typed::kBool) {
                fCurrentBlock->push(new FIRBasicInstruction<T>(gBinOpTable[inst->fOpcode]->fInterpIntInst));
            } else {
                assert(false);
            }
            
            fTypingVisitor.visit(inst);
        }
        
        virtual void visit(CastNumInst* inst) 
        {
            inst->fInst->accept(this);
       
            // Typing the argument
            inst->fInst->accept(&fTypingVisitor);
            assert(fTypingVisitor.fCurType != Typed::kNoType);
            
            if (inst->fType->getType() == Typed::kInt) {
                fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kCastInt));
            } else if (isInternalRealType(inst->fType->getType()) && (fTypingVisitor.fCurType == Typed::kFloatMacro)) {
                // We assume that kFloatMacro and internal float are the same for now, so no cast...
            } else if (isInternalRealType(fTypingVisitor.fCurType) && (inst->fType->getType() == Typed::kFloatMacro)) {
                // We assume that kFloatMacro and internal float are the same for now, so no cast...
            } else {
                fCurrentBlock->push(new FIRBasicInstruction<T>(FIRInstruction::kCastReal));
            }
            fTypingVisitor.visit(inst);
        }

        // Function call
        virtual void visit(FunCallInst* inst)
        {
            // Compile args
            //list<ValueInst*>::const_iterator it = beg;
            fCurrentBlock->push(new FIRBasicInstruction<T>(gMathLibTable[inst->fName]));
        }
        virtual void visit(RetInst* inst) { printf("visit(RetInst* inst)\n");}
        virtual void visit(DropInst* inst) {}

        // Conditionnal
        virtual void visit(Select2Inst* inst)
        {
            printf("visit(Select2Inst* inst)\n");
            fTypingVisitor.visit(inst);
            
            // Compile condition
            inst->fCond->accept(this);
            
            // Keep current block
            FIRBlockInstruction<T>* previous = fCurrentBlock;
            
            // Compile 'then' in a new block
            FIRBlockInstruction<T>* then_block = new FIRBlockInstruction<T>();
            fCurrentBlock = then_block;
            inst->fThen->accept(this);
            // Add kHalt in block
            then_block->push(new FIRBasicInstruction<T>(FIRInstruction::kHalt));
            
            // Compile 'else' in a new block
            FIRBlockInstruction<T>* else_block = new FIRBlockInstruction<T>();
            fCurrentBlock = else_block;
            inst->fElse->accept(this);
            // Add kHalt in block
            else_block->push(new FIRBasicInstruction<T>(FIRInstruction::kHalt));
            
            // Compile 'select'
            previous->push(new FIRBasicInstruction<T>((isIntType(fTypingVisitor.fCurType) ? FIRInstruction::kSelectInt : FIRInstruction::kSelectReal),
                                                    0, 0, 0, 0,
                                                    then_block, else_block));
                                                           
            // Restore current block
            fCurrentBlock = previous;
        }
    
        virtual void visit(IfInst* inst)
        {
            // Compile condition
            inst->fCond->accept(this);
            
            // Keep current block
            FIRBlockInstruction<T>* previous = fCurrentBlock;
            
            // Compile 'then' in a new block
            FIRBlockInstruction<T>* then_block = new FIRBlockInstruction<T>();
            fCurrentBlock = then_block;
            inst->fThen->accept(this);
            // Add kHalt in block
            then_block->push(new FIRBasicInstruction<T>(FIRInstruction::kHalt));
            
            // Possibly compile 'else' in a new block
            FIRBlockInstruction<T>* else_block = 0;
            if (inst->fElse->fCode.size() > 0) {
                else_block = new FIRBlockInstruction<T>();
                fCurrentBlock = else_block;
                inst->fElse->accept(this);
                // Add kHalt in block
                else_block->push(new FIRBasicInstruction<T>(FIRInstruction::kHalt));
            }
            
            // Compile 'if'
            previous->push(new FIRBasicInstruction<T>(FIRInstruction::kIf, 0, 0, 0, 0, then_block, else_block));
            
            // Restore current block
            fCurrentBlock = previous;
        }
    
        virtual void visit(SwitchInst* inst) {}

        // Loops
        virtual void visit(ForLoopInst* inst) 
        {
            // Compile loop variable declaration
            inst->fInit->accept(this);
           
            // Keep current block
            FIRBlockInstruction<T>* previous = fCurrentBlock;
            
            // Compile 'loop code' in a new block
            FIRBlockInstruction<T>* loop_block = new FIRBlockInstruction<T>();
            fCurrentBlock = loop_block;
            inst->fCode->accept(this);
            // Add kHalt in block
            loop_block->push(new FIRBasicInstruction<T>(FIRInstruction::kHalt));
           
            // Push Loop instruction
            pair<int, Typed::VarType> tmp = fFieldTable[inst->getVariableName()];
            previous->push(new FIRBasicInstruction<T>(FIRInstruction::kLoop, inst->getVariableCount(), 0, tmp.first, 0, loop_block, 0));
            
            // Restore current block
            fCurrentBlock = previous;
        }
        
        virtual void visit(WhileLoopInst* inst) {}

};

#endif