{-# LANGUAGE TupleSections #-}
{-# OPTIONS_GHC -Wno-unused-do-bind #-}

module Parsing (parseProgram) where

import Data.Functor
import qualified Data.Map as M
import Data.Maybe
import Data.Void
import Program
import Text.Megaparsec hiding (Label, label)
import Text.Megaparsec.Char
import Prelude hiding (lines)

data Line = Instr Instr | Label Label
  deriving (Show)

lineToInstr :: Line -> Maybe Instr
lineToInstr (Instr i) = Just i
lineToInstr _ = Nothing

lineToLabel :: Line -> Maybe Label
lineToLabel (Label l) = Just l
lineToLabel _ = Nothing

type Parser = Parsec Void String

parseRegister :: Parser Reg
parseRegister =
  hspace
    *> choice
      [ string "rz" $> RZ,
        string "r1" $> R1,
        string "r2" $> R2,
        string "r3" $> R3,
        string "r4" $> R4,
        string "r5" $> R5,
        string "r6" $> R6,
        string "r7" $> R7
      ]
    <* hspace

parseImmediate :: Parser Imm
parseImmediate = hspace *> (read <$> some digitChar) <* hspace

parseLabel :: Parser Label
parseLabel = hspace *> some letterChar <* hspace

parseInstruction :: Parser Instr
parseInstruction =
  hspace
    *> choice
      [ string "hlt" $> Halt,
        regOrImm "mov" MovReg MovImm,
        regOrImm "shl" ShlReg ShlImm,
        regOrImm "shr" ShrReg ShrImm,
        regOrImm "add" AddReg AddImm,
        regOrImm "sub" SubReg SubImm,
        regOrImm "mul" MulReg MulImm,
        regOrImm "div" DivReg DivImm,
        do
          string "ld"
          dest <- parseRegister
          char ','
          uncurry (Load dest) <$> addr,
        do
          string "st"
          dest <- addr
          char ','
          uncurry Store dest <$> parseRegister,
        string "jmp" *> (Jmp <$> parseLabel),
        br "blt" Blt,
        br "ble" Ble,
        br "bgt" Bgt,
        br "bge" Bge,
        br "beq" Beq,
        br "bne" Bne
      ]
    <* hspace
  where
    regOrImm :: String -> (Reg -> Reg -> Instr) -> (Reg -> Imm -> Instr) -> Parser Instr
    regOrImm mnemonic reg imm = do
      string mnemonic
      hspace
      lhs <- parseRegister
      char ','
      (reg lhs <$> try parseRegister) <|> (imm lhs <$> parseImmediate)

    addr :: Parser (Reg, Reg)
    addr = between (hspace *> char '[') (hspace *> char ']' *> hspace) $ do
      base <- parseRegister
      offset <- (char '+' *> parseRegister) <|> pure RZ
      pure (base, offset)

    br :: String -> (Reg -> Label -> Instr) -> Parser Instr
    br mnemonic constructor = do
      string mnemonic
      hspace
      reg <- parseRegister
      char ','
      constructor reg <$> parseLabel

parseRawProgram :: Parser [Line]
parseRawProgram = do
  space
  res <-
    sepEndBy
      ( (Instr <$> parseInstruction)
          <|> (Label <$> parseLabel <* char ':' <* hspace)
      )
      (some (newline *> hspace))
  eof
  pure res

indexLabels :: [Line] -> LabelMap
indexLabels lines =
  foldl
    (\labels (label, i) -> M.insert label (i - fromIntegral (M.size labels)) labels)
    M.empty
    rawIndexedLabels
  where
    rawIndexedLabels = mapMaybe (\(l, i) -> (,i) <$> lineToLabel l) (lines `zip` [0 ..])

parseProgram :: Parser Program
parseProgram = do
  raw <- parseRawProgram
  pure $ Program (mapMaybe lineToInstr raw) (indexLabels raw)
