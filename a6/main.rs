// get arguments from command line
use std::env;
use std::thread;
use std::sync::mpsc;

/*
print the number of partitions and the size of each partition
@param vs - A vector of vectors
*/
fn print_partition_info(vs: &Vec<Vec<usize>>){
	println!("Number of partitions = {}", vs.len());
	for i in 0..vs.len() {
		println!("\tsize of partition {} = {}", i, vs[i].len());
	}
}

/*
Create a vector with integers from 0 to num_elements - 1
@param num_elements How many integers to generate
@return A vector with integers from 0 to (num_elements - 1)
*/
fn generate_data(num_elements: usize) -> Vec<usize> {
	let mut v : Vec<usize> = Vec::new();
	for i in 0..num_elements {
		v.push(i);
	}	
	v
}

/*
Partition the data in the vector v into 2 vectors
@param v Vector of integers
@return A vector that contains 2 vectors of integers
*/
fn partition_data_in_two(v: &Vec<usize>) -> Vec<Vec<usize>> {
	let partition_size = v.len() / 2;
	// create a vector of vectors of integers
	let mut xs : Vec<Vec<usize>> = Vec::new();

	// add the first half of the integers from v to x1 
	let mut x1 : Vec<usize> = Vec::new();
	for i in 0..partition_size {
		x1.push(v[i]);
	}
	// push x1 to the returned vector
	xs.push(x1);

	// perform the same steps for the remaining half of the
	// input vectors
	let mut x2 : Vec<usize> = Vec::new();
	for i in partition_size..v.len() {
		x2.push(v[i]);
	}
	xs.push(x2);
	
	xs
}

/*
Sum up all the integers in the given vector
@param v Vector of integers
@return Sum of integers in v
Note: this function has the same code as the reduce_data
			function. Don't change the code of map_data of reduce_data.
*/
fn map_data(v : &Vec<usize>) -> usize {
	let mut sum = 0;
	for i in v {
		sum += i;		
	}
	sum
}

/*
Sum up all the integers in the given vector
@param v Vector of integers
@return Sum of integers in v
*/
fn reduce_data(v : &Vec<usize>) -> usize {
	let mut sum = 0;
	for i in v {
		sum += i;
	}
	sum
}

// Turn this into a multi-threaded map-reduce program
fn main() {

	let args: Vec<String> = env::args().collect();
	if args.len() != 3 {
		println!("ERROR: Usage {} num_partitions num_elements", args[0]);
	}

	let num_partitions : usize = args[1].parse().unwrap();
	let num_elements : usize = args[2].parse().unwrap();

	// Check command input
	if num_partitions < 1 {
		println!("ERROR: num_partitions must be at least 1");
		return;
	}
	if num_elements < num_partitions {
		println!("ERROR: num_elements cannot be smaller than num_partitions");
		return;
	} //

	// Get the data and create a vector
	let v = generate_data(num_elements);

	// split v into two vectors
	let xs = partition_data_in_two(&v);

	print_partition_info(&xs);
	let mut intermediate_sums : Vec<usize> = Vec::new();

	////
	// MAP STEP: Process each partition
	//
	// CHANGE CODE START: Don't change any code above this line

	// Change the following code to create 2 threads that run
	// concurrently and each of which uses map_data() function
	// to process one of the two partitions

	// split xs into two vectors so there is uniqueness
	// for each thread input
	let mut v1: Vec<usize> = Vec::new();
	let mut v2: Vec<usize> = Vec::new();

	for i in 0..xs[0].len() {
		v1.push(xs[0][i]);
	}
	for i in 0..xs[1].len() {
		v2.push(xs[1][i]);
	}
	
	// create the two threads
	let t1 = thread::spawn(move || {
		let res = map_data(&v1);			
		res
	});
	let t2 = thread::spawn(move || {
		let res = map_data(&v2);
		res
	});

	// accumulate the values returned from each thread
	let r1 = t1.join().unwrap();
	let r2 = t2.join().unwrap();

	intermediate_sums.push(r1);
	intermediate_sums.push(r2);		

	// CHANGE CODE END: Don't change any code below this line until
	// the next CHANGE CODE comment

	println!("Intermediate sums = {:?}", intermediate_sums);

	// REDUCE STEP: Process the intermediate result to produce
	// the final result
	let sum = reduce_data(&intermediate_sums);
	println!("Sum = {}", sum);	

	////
	// CHANGE CODE: Add code that does the following
	/*	
	1. Calls partition_data to partition the data into equal partitions
	2. Calls print_partition_info to print info in the partitions that 
			have been created
	3. Creates one thread per partition and uses each thread to 
			concurrently process one partition
	4. Collects the intermediate sums from all the threads
	5. Prints information about the intermediate sums
	6. Calls reduce_data to process the intermediate sums
	7. Prints the final sum computed by reduce_data
	*/	
	let vx = partition_data(num_partitions, &v);
	let mut sums : Vec<usize> = Vec::new();

	print_partition_info(&vx);

	// create enough threads to process each partition
	let (tx, rx) = mpsc::channel();
	let tx1 = tx.clone();
	// send
	thread::spawn(move || {
		for x in vx {
			let result = map_data(&x);
			tx1.send(result).unwrap();
		}
		drop(tx);
	});
	// recv
	for received in rx {
		sums.push(received);
	}

	// accumulate the values from each thread
	let total = reduce_data(&sums);
	println!("Intermediate sums = {:?}", sums);
	println!("Sum = {}", total);	
}

////
// CHANGE CODE: code this function
// Note: Don't change the signature of this function
/*
Partitions the data into a number of partitions such that
- the returned partitions contain all elements that are in the 
	input vector
- if num_elements is a multiple of num_partitions, then all partitions
	must have equal number of elements
- if num_elements is not a multiple of num_partitions, some partitions
	can have one more element than other partitions

@param num_partitions The number of partitions to create
@param v The data to be partitioned
@return A vector that contains vectors of integers
*/
fn partition_data(num_partitions: usize, v: &Vec<usize>) -> Vec<Vec<usize>> {
	// create the container for the vector of vectors
	let mut vv : Vec<Vec<usize>> = Vec::new();	
	let mut j = 0;
	let mut pos = 0;

	// create the number of partitions vectors used for each thread input
	while j < num_partitions {
		let nv : Vec<usize> = Vec::new();
		vv.push(nv);	
		j += 1;
	}

	// fill the vectors with the contents of v	
	for i in 0..v.len() {
		if pos == num_partitions { pos = 0; }
		vv[pos].push(v[i]);
		pos += 1;
	}	

	vv	
}
