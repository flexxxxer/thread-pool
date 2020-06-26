# Thread Pool | Fixed-size array of threads

## This thread-pool implementation allows you to:
- perform tasks without the cost of creating and destroying threads
- allows you to perform tasks in different modes: asynchronous and parallel
- performing functions with any number of parameters

## Features:
- pooling on first initialization (singleton)
- all tasks all tasks are performed on pool

## Usage
##### Executing functuions:
``` cpp
int sum(int a, int b) {
	return a + b;
}

thread_pool& pool = thread_pool::instance();
int result = pool.async(sum, 5, 2); // result eq to 7
```
##### Executing lambda:
``` cpp
int c = 5;
auto func = [c](int a, int b) {
	return a + b + c;
};
	
thread_pool& pool = thread_pool::instance();
int result = pool.async(func, 1, 2); // result eq to 8
```
##### Executing object method:
``` cpp
class foo {
	int c = 0;
public:
	int get_c() const {
		return c;
	}
	void bar(int a, int b) {
		c = a + b;
	}
};
	
foo obj;
thread_pool& pool = thread_pool::instance();
pool.async(&foo::bar, &obj, 2, 3);
int c_value = obj.get_c(); // c_value eq to 5
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)
