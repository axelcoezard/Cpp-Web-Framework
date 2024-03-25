import { useState } from 'react';
import './App.css';

const App = () => {
	const [value, setValue] = useState('');

	const handleClick = () => {
		fetch('http://localhost:8080/planning', {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ value })
		})
			.then(response => response.json())
			.then(data => setValue(data.message));

	}

	const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
		setValue(e.target.value);
	}

	return (
		<div className="App">
			<h1>{value}</h1>
			<input type="text" onChange={handleChange}/>
			<button onClick={handleClick}>Click me</button>
		</div>
	);
}

export default App;
