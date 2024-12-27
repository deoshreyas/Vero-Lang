# Vero Programming Language Syntax Documentation

## 1. Introduction

The Vero programming language is designed with simplicity in mind. This documentation provides an overview of its syntax, covering fundamental concepts, data types, control structures, and common operations.

---

## 2. Basic Syntax

### 2.1 Printing

The `print` function is used to display output:

```vero
print("Hello world!")
```

---

### 2.2 Comments

Single-line comments start with `#`:

```vero
# This is a comment
```

---

## 3. Variables

### 3.1 Variable Declaration

Variables are declared using the `let` keyword:

```vero
let var = 1; # 1 is true, 0 is false
```

Please note that you need to use the same syntax when updating variables as well:
```vero
let a = 1
let a = a + 1
```

### 3.2 Data Types

Vero supports multiple data types:

- **Integer**:

  ```vero
  let integer = 1
  ```

- **Floating-point**:

  ```vero
  let floating = 1.0
  ```

- **List**:

  ```vero
  let list = [1, 2, 3, 4]
  ```

- **String**:

  ```vero
  let string = "Hello!"
  ```

---

## 4. Mathematical Operations

Basic arithmetic operations are supported. For example:

```vero
print(var + 8)
```

---

## 5. Control Structures

### 5.1 If-Else Statements

Conditional branching is achieved using `if`, `elif`, and `else` keywords:

```vero
if (var == 0) {
	print(var, " is 0")
} elif (var == 10) {
	print(var, " is 10")
} else {
	print(var, " is ", var)
}
```

### 5.2 Loops

#### 5.2.1 For Loops

Iterate over a range using the `for` loop:

```vero
for i = 0 to 6 step 0.5 {
	print(i)
}
```

#### 5.2.2 While Loops

Execute a block of code as long as a condition is true:

```vero
let a = 1

while (a <= 10) {
	if (a == 5) {
		let a = a + 1
		continue
	}
	print(a)
	let a = a + 1
}
```

---

## 6. Functions

Define reusable blocks of code with functions:

```vero
function greet(name) {
	print("Hello, ", name, "!")
}

greet("Alice")
```

---

## 7. Additional Features

### 7.1 Logical Operators

Vero supports standard logical operators:

- **Equality**: `==`
- **Not Equal**: `!=`
- **Greater than:** `>`
- **Less than:** `<`
- **Greater than or equal to:** `>=`
- **Less than or equal to:** `<=`
- **And**: `and`
- **Or**: `or`
- **Not:**  `not`

Example:

```vero
if (var == 1 and integer != 0) {
	print("Condition met")
}
```

### 7.2 Lists

Lists allow storing sequences of values:

```vero
let myList = [10, 20, 30]

for i=0 to len(myList) {
	print(myList / i) # The / syntax is used to access elements
}
```

The following operations are supported with lists:

1. **Concatenation**:
```vero
let myList = myList + [40, 50] # [10, 20, 30, 40, 50]
```

2. **Replication**
```vero
let myList = myList * 1 
# (* 1 means the list variable will now hold [10, 20, 30, 10, 20, 30]) 
```

---

## 8. Error Handling

Vero has a very robust error handling system that should meet most of your needs. 

1. **Illegal Character Error:**
   ```
   Illegal Character: '`'
   File:'<stdin>', Line:1

   `
   ^
   ```
2. **Syntax Error:**
   ```
   Invalid Syntax: Expected 'let', INT, FLOAT, IDENTIFIER, 'if', 'while', 'func', '+', '-', '[' or '('
   File:'<stdin>', Line:1

   let l = step
           ^
   ```
3. **Expected Character Error:**
   ```
   Expected Character: '=' (after '!')
   File:'<stdin>', Line:1

   if (5!) {
        ^
   ```
4. **Runtime Error (with traceback):**
   ```
   Traceback (most recent call last):
     File '<stdin>', line 5, in <div>
     File '<stdin>', line 1, in <program>
   Runtime Error: Division by zero

   div(1, 0)
   ^
   ```
