import { useEffect, useState } from 'react';
import './App.css';

const App = () => {
	const [receivedValue, setReceivedValue] = useState('');
	const [value, setValue] = useState('');

	useEffect(() => {
		fetch('http://localhost:8080/planning')
			.then(response => response.json())
			.then(data => setReceivedValue(data.value));
	}, []);

	const handleClick = () => {
		fetch('http://localhost:8080/planning', {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ value })
		})
			.then(response => response.json())
			.then(data => setReceivedValue(data.value));
	}

	const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
		setValue(e.target.value);
	}

	return (
		<div className="App">
			<h1>{receivedValue}</h1>
			<input type="text" onChange={handleChange}/>
			<button onClick={handleClick}>Click me</button>
		</div>
	);
}

export default App;
