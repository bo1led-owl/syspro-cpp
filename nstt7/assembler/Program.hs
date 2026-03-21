module Program where

import Data.Binary
import qualified Data.Map as M

data Reg = RZ | R1 | R2 | R3 | R4 | R5 | R6 | R7
  deriving (Show)

type Imm = Word16

type Label = String

data Instr
  = Halt
  | MovReg Reg Reg
  | MovImm Reg Imm
  | ShlReg Reg Reg
  | ShlImm Reg Imm
  | ShrReg Reg Reg
  | ShrImm Reg Imm
  | AddReg Reg Reg
  | AddImm Reg Imm
  | SubReg Reg Reg
  | SubImm Reg Imm
  | MulReg Reg Reg
  | MulImm Reg Imm
  | DivReg Reg Reg
  | DivImm Reg Imm
  | Load Reg Reg Reg
  | Store Reg Reg Reg
  | Jmp Label
  | Blt Reg Label
  | Ble Reg Label
  | Bgt Reg Label
  | Bge Reg Label
  | Beq Reg Label
  | Bne Reg Label
  deriving (Show)

type LabelMap = M.Map Label Imm

data Program = Program [Instr] LabelMap
